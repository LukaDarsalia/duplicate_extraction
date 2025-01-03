# Text Duplicate Finder

A high-performance C++ library and command-line tool for finding duplicate text across documents using suffix arrays. The project includes full UTF-8 support, efficient document storage, and SQLite integration for managing large document collections.

## Features

- Fast duplicate text detection using suffix array algorithms
- Complete UTF-8 support for handling text in any language
- Efficient document storage and management
- SQLite database integration for document persistence
- Parquet file support through Python converter
- Customizable minimum length threshold for duplicate detection
- JSON output format for easy integration with other tools
- Comprehensive test coverage with Google Test framework

## Prerequisites

- CMake 3.14 or higher
- C++17 compatible compiler
- SQLite3 development libraries
- Python 3.x (for Parquet conversion)
- pandas (for Parquet handling)
- pyarrow (for Parquet support)

## Building the Project

1. Clone the repository:
```bash
git clone https://github.com/yourusername/text-duplicate-finder.git
cd text-duplicate-finder
```

2. Create a build directory:
```bash
mkdir build && cd build
```

3. Configure and build:
```bash
cmake ..
make
```

4. Run tests:
```bash
ctest --verbose
```

## Usage

### Command Line Interface

The main program accepts the following arguments:

```bash
./main [-v|--verbose] <database_path> <output_json_path> <domain> <threshold>
```

Parameters:
- `-v|--verbose`: Optional flag for verbose output
- `database_path`: Path to SQLite database containing documents
- `output_json_path`: Path where to save the JSON output
- `domain`: Domain to filter documents (e.g., "example.com")
- `threshold`: Minimum length of duplicate text to report

Example:
```bash
./main data.db output.json example.com 50
```

### Converting Parquet to SQLite

For working with Parquet files, use the provided Python converter:

```bash
python3 parquet_to_sqlite.py input.parquet output.db
```

## Project Structure

The project follows a modular design with clear separation of concerns:

### Core Components

- `text_processing/`: Core text processing functionality
  - `utf8_handler`: UTF-8 string handling
  - `suffix_array_builder`: Abstract interface for suffix array construction
  - `naive_suffix_builder`: O(n log n) suffix array implementation
  - `duplicate_finder`: Main duplicate detection logic

- `data/`: Data management
  - `document_store`: Efficient document storage and retrieval
  - `duplicate_match`: Match result representation

- `sql/`: Database integration
  - `sql_handler`: SQLite database operations

### Design Patterns Used

- Factory Method: For suffix array builder creation
- Strategy Pattern: For different suffix array construction algorithms
- Iterator Pattern: For UTF-8 string traversal
- RAII: For resource management (SQLite connections, file handles)

## Algorithm Details

The duplicate detection algorithm works in several steps:

1. Documents are concatenated with separator characters
2. A suffix array is built using the O(n log n) algorithm
3. The Longest Common Prefix (LCP) array is constructed using Kasai's algorithm
4. Common substrings are identified by processing the LCP array
5. Results are filtered by minimum length and document boundaries

## Performance Considerations

- Uses chunked processing for large files
- Implements memory-efficient UTF-8 handling
- Optimizes database operations with prepared statements
- Employs efficient string concatenation strategies
- Uses binary search for document position lookup

## Testing

The project includes extensive unit tests using Google Test framework:

- UTF-8 string handling tests
- Suffix array construction tests
- Document store tests
- Duplicate finder tests
- SQLite integration tests

Run all tests with:
```bash
cd build && ctest --verbose
```

## Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Suffix array implementation inspired by the CP-Algorithms website
- UTF-8 handling based on the Unicode Standard Version 14.0
- SQLite integration influenced by SQLite documentation and best practices
