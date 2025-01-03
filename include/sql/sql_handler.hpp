#ifndef DB_SQLITE_HANDLER_HPP
#define DB_SQLITE_HANDLER_HPP

#include <string>
#include <vector>
#include <sqlite3.h>
#include "data/document_store.hpp"

namespace text_processing {

/**
 * @brief Custom exception class for SQLite operations
 */
class SQLiteError : public std::runtime_error {
public:
    explicit SQLiteError(const std::string& message) : std::runtime_error(message) {}
};

/**
 * @brief Handler for SQLite database operations with focus on document grouping
 */
class SQLiteHandler {
public:
    /**
     * @brief Constructor that initializes database connection
     *
     * @param db_path Path to SQLite database file
     * @param verbose If true, will print progress
     * @throw SQLiteError if connection cannot be established
     */
    explicit SQLiteHandler(const std::string& db_path, bool verbose = false);

    /**
     * @brief Destructor ensuring proper cleanup of database connection
     */
    ~SQLiteHandler();

    // Prevent copying, allow moving
    SQLiteHandler(const SQLiteHandler&) = delete;
    SQLiteHandler& operator=(const SQLiteHandler&) = delete;
    SQLiteHandler(SQLiteHandler&&) noexcept;
    SQLiteHandler& operator=(SQLiteHandler&&) noexcept;

    /**
     * @brief Explicitly close the database connection
     */
    void close();

    /**
     * @brief Creates DocumentStore from content filtered by a column value
     *
     * @param table_name Name of the table to query
     * @param filter_column Name of the column to filter by (e.g., "domain")
     * @param content_column Name of the column containing text content
     * @param filter_value Value to filter the rows by
     * @return DocumentStore Store containing concatenated documents matching the filter
     * @throw SQLiteError if query fails or columns don't exist
     */
    DocumentStore createDocumentStore(
        const std::string& table_name,
        const std::string& filter_column,
        const std::string& content_column,
        const std::string& filter_value
    );

    /**
     * @brief Check if table and columns exist
     *
     * @param table_name Table to check
     * @param columns Vector of column names to verify
     * @return Pair containing boolean indicating existence and missing table/column name
     */
    std::pair<bool, std::string> validateTableAndColumns(
        const std::string& table_name,
        const std::vector<std::string>& columns
    );

    /**
     * @brief Update a single row in the specified table
     *
     * @param table_name Name of the table to update
     * @param row_id ID of the row to update
     * @param column_name Name of the column to update
     * @param new_value New value to set for the specified column
     * @throw SQLiteError if update fails
     */
    void updateRow(
        const std::string& table_name,
        int64_t row_id,
        const std::string& column_name,
        const std::string& new_value
    );

private:
    sqlite3* db_connection_{};  ///< SQLite database connection handle
    bool verbose_;

    /**
     * @brief Execute SQLite query and process results
     *
     * @tparam Callback Type of the callback function
     * @param query SQL query string
     * @param callback Function to process each row
     * @throw SQLiteError if query execution fails
     */
    template <typename Callback>
    void executeQuery(const std::string& query, Callback callback);

    /**
     * @brief Sanitize input to prevent SQL injection
     *
     * @param input String to sanitize
     * @return std::string Sanitized string
     */
    static std::string sanitizeInput(const std::string& input);

    /**
     * @brief Build SQL query for document selection
     *
     * Builds a SELECT query to fetch the 'content' column filtered by the provided 'filter_value'.
     * The query is of the form:
     *   SELECT <content_column>
     *   FROM <table_name>
     *   WHERE <filter_column> = '<filter_value>'
     *   @throw SQLiteError if query is malformed
     */
    static std::string buildQuery(
        const std::string& table_name,
        const std::string& filter_column,
        const std::string& content_column,
        const std::string& filter_value
    );
};

} // namespace text_processing

#endif // DB_SQLITE_HANDLER_HPP