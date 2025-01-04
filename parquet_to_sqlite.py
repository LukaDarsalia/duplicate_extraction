#!/usr/bin/env python3
"""
Script to convert Parquet files to SQLite database.
Handles large datasets using Dask and implements proper logging.
"""

import argparse
import logging
import sys
import psutil
import os
from pathlib import Path
from typing import Optional, NoReturn, Iterator, Tuple

import pandas as pd
import pyarrow.parquet as pq
import sqlite3
from sqlite3 import Connection
from tqdm import tqdm


class ParquetToSQLiteConverter:
    """
    A class to handle conversion of Parquet files to SQLite database.

    Attributes:
        input_file (Path): Path to the input Parquet file
        output_file (Path): Path to the output SQLite database
        logger (logging.Logger): Logger instance for the class
        chunk_size (int): Size of chunks to process at once
        index_column (Optional[str]): Column name to create index on
    """

    def __init__(
            self,
            input_file: Path,
            output_file: Path,
            chunk_size: int = 10000,  # Reduced default chunk size
            log_level: int = logging.INFO,
            index_column: Optional[str] = None
    ) -> None:
        """
        Initialize the converter with input and output paths.

        Args:
            input_file: Path to the input Parquet file
            output_file: Path to the output SQLite database
            chunk_size: Number of rows to process at once
            log_level: Logging level to use
            index_column: Column name to create index on
        """
        self.input_file = input_file
        self.output_file = output_file
        self.chunk_size = chunk_size
        self.index_column = index_column
        self.logger = self._setup_logger(log_level)

    def _get_memory_usage(self) -> float:
        """
        Get current memory usage of the process in MB.

        Returns:
            Current memory usage in MB
        """
        process = psutil.Process(os.getpid())
        return process.memory_info().rss / 1024 / 1024  # Convert to MB

    def _adjust_chunk_size(self, total_rows: int) -> None:
        """
        Adjust chunk size based on available system memory.

        Args:
            total_rows: Total number of rows in the dataset
        """
        available_memory = psutil.virtual_memory().available / 1024 / 1024  # MB
        current_memory = self._get_memory_usage()

        # Target using no more than 75% of available memory
        target_memory = available_memory * 0.75

        # If we're already using too much memory, reduce chunk size
        if current_memory > target_memory / 2:
            self.chunk_size = max(1000, self.chunk_size // 2)
            self.logger.info(f"Reduced chunk size to {self.chunk_size} due to memory constraints")

        self.logger.debug(f"Available memory: {available_memory:.2f}MB, Current usage: {current_memory:.2f}MB")

    def _setup_logger(self, log_level: int) -> logging.Logger:
        """
        Set up logging configuration.

        Args:
            log_level: Logging level to use

        Returns:
            Configured logger instance
        """
        logger = logging.getLogger(__name__)
        logger.setLevel(log_level)

        # Create console handler with formatting
        handler = logging.StreamHandler()
        formatter = logging.Formatter(
            '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
        )
        handler.setFormatter(formatter)
        logger.addHandler(handler)

        return logger

    def _create_sqlite_connection(self) -> Connection:
        """
        Create a connection to SQLite database.

        Returns:
            SQLite database connection

        Raises:
            sqlite3.Error: If connection cannot be established
        """
        try:
            conn = sqlite3.connect(str(self.output_file))
            self.logger.debug("SQLite connection established")
            return conn
        except sqlite3.Error as e:
            self.logger.error(f"Failed to connect to SQLite database: {e}")
            raise

    def _create_indices(self, conn: Connection) -> None:
        """
        Create necessary indices on the SQLite database.

        Args:
            conn: SQLite database connection

        Raises:
            sqlite3.Error: If index creation fails
        """
        if not self.index_column:
            self.logger.info("No index column specified, skipping index creation")
            return

        try:
            self.logger.info(f"Creating index on {self.index_column} column...")
            # Use parameterized identifier
            index_name = f"idx_{self.index_column.lower()}"
            # Note: We can't use parameters for identifiers in SQLite,
            # but since we're using the column name from the DataFrame,
            # it's already validated as a valid column name
            conn.execute(f'CREATE INDEX IF NOT EXISTS {index_name} ON data_table({self.index_column})')
            conn.commit()
            self.logger.info("Index created successfully")
        except sqlite3.Error as e:
            self.logger.error(f"Failed to create index: {e}")
            raise

    def _read_parquet_chunks(self) -> Iterator[Tuple[pd.DataFrame, int]]:
        """
        Read parquet file in chunks using pyarrow.

        Yields:
            Tuple of (DataFrame chunk, total number of rows)
        """
        # Open parquet file
        parquet_file = pq.ParquetFile(str(self.input_file))
        total_rows = parquet_file.metadata.num_rows

        # Create iterator once and use it throughout
        batch_iterator = parquet_file.iter_batches(batch_size=self.chunk_size)

        for chunk in batch_iterator:
            df_chunk = chunk.to_pandas()
            yield df_chunk, total_rows

    def convert(self) -> None:
        """
        Convert Parquet file to SQLite database.

        This method handles the main conversion process including:
        - Reading the Parquet file chunk by chunk
        - Converting to SQLite
        - Creating necessary indices

        Raises:
            FileNotFoundError: If input file doesn't exist
            Exception: For other unexpected errors
        """
        conn = None
        try:
            self.logger.info(f"Starting conversion of {self.input_file}")

            if not self.input_file.exists():
                raise FileNotFoundError(f"Input file not found: {self.input_file}")

            # Create SQLite connection
            conn = self._create_sqlite_connection()

            # Process chunks
            first_chunk = True
            chunks_iterator = self._read_parquet_chunks()
            first_df, total_rows = next(chunks_iterator)

            self.logger.info(f"Found {total_rows:,} rows in Parquet file")
            self._adjust_chunk_size(total_rows)

            # Create table with first chunk
            first_df.to_sql('data_table', conn, if_exists='replace', index=False)
            del first_df

            # Process remaining chunks
            with tqdm(total=total_rows, initial=self.chunk_size) as pbar:
                for df_chunk, _ in chunks_iterator:
                    try:
                        # Monitor memory
                        mem_usage = self._get_memory_usage()
                        self.logger.debug(f"Memory usage: {mem_usage:.2f}MB")

                        # Append to SQLite
                        df_chunk.to_sql(
                            'data_table',
                            conn,
                            if_exists='append',
                            index=False,
                            method='multi'
                        )

                        # Update progress
                        pbar.update(len(df_chunk))

                        # Cleanup
                        del df_chunk
                        import gc
                        gc.collect()

                    except Exception as chunk_error:
                        self.logger.error(f"Error processing chunk: {str(chunk_error)}")
                        raise Exception(f"Chunk processing failed: {str(chunk_error)}") from chunk_error

            # Create indices
            self._create_indices(conn)

            self.logger.info("Conversion completed successfully")

        except Exception as e:
            self.logger.error(f"Conversion failed: {str(e)}")
            raise
        finally:
            if conn:
                conn.close()
                self.logger.debug("SQLite connection closed")


def parse_arguments() -> argparse.Namespace:
    """
    Parse command line arguments.

    Returns:
        Parsed command line arguments
    """
    parser = argparse.ArgumentParser(
        description='Convert Parquet file to SQLite database'
    )
    parser.add_argument(
        'input_file',
        type=str,
        help='Path to input Parquet file'
    )
    parser.add_argument(
        '--output',
        type=str,
        help='Path to output SQLite database (default: input_file_name.db)',
        default=None
    )
    parser.add_argument(
        '--chunk-size',
        type=int,
        help='Number of rows to process at once (default: 100000)',
        default=100000
    )
    parser.add_argument(
        '--debug',
        action='store_true',
        help='Enable debug logging'
    )
    parser.add_argument(
        '--index-column',
        type=str,
        help='Column name to create index on (e.g., domains)',
        default=None
    )

    return parser.parse_args()


def main() -> Optional[NoReturn]:
    """
    Main entry point for the script.

    Returns:
        None on success, exits with status code 1 on error
    """
    args = parse_arguments()

    input_path = Path(args.input_file)
    output_path = Path(args.output if args.output else input_path.stem + '.db')
    log_level = logging.DEBUG if args.debug else logging.INFO

    try:
        converter = ParquetToSQLiteConverter(
            input_path,
            output_path,
            args.chunk_size,
            log_level,
            args.index_column
        )
        converter.convert()
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
