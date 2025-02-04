#include "sql/sql_handler.hpp"

#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <regex>

bool isValidName(const std::string& name) {
    // Regex to match valid table/column names: alphanumeric and underscores only
    static const std::regex valid_name_regex("^[A-Za-z_][A-Za-z0-9_]*$");

    // Check if the name matches the regex
    return std::regex_match(name, valid_name_regex);
}

namespace text_processing {

SQLiteHandler::SQLiteHandler(const std::string& db_path, bool verbose) {
    verbose_ = verbose;
    if (sqlite3_open(db_path.c_str(), &db_connection_) != SQLITE_OK) {
        throw SQLiteError("Failed to open database: " + db_path);
    }
}

SQLiteHandler::~SQLiteHandler() {
    close();
}

SQLiteHandler::SQLiteHandler(SQLiteHandler&& other) noexcept
    : db_connection_(other.db_connection_) {
    other.db_connection_ = nullptr;
    verbose_ = other.verbose_;
}

SQLiteHandler& SQLiteHandler::operator=(SQLiteHandler&& other) noexcept {
    if (this != &other) {
        close();
        db_connection_ = other.db_connection_;
        other.db_connection_ = nullptr;
        verbose_ = other.verbose_;
    }
    return *this;
}

void SQLiteHandler::close() {
    if (db_connection_) {
        sqlite3_close(db_connection_);
        db_connection_ = nullptr;
        if (verbose_) std::cout << "Closing database." << std::endl;
    }
}

DocumentStore SQLiteHandler::createDocumentStore(
    const std::string& table_name,
    const std::string& filter_column,
    const std::string& content_column,
    const std::string& filter_value,
    const std::string& separator
) {
    // Get approximate total size first
    std::string size_query = "SELECT COUNT(*), SUM(LENGTH(" + content_column + ")) "
                            "FROM " + table_name +
                            " WHERE " + filter_column + " = '" + filter_value + "'";

    size_t doc_count = 0;
    size_t total_size = 0;

    executeQuery(size_query, [&](sqlite3_stmt* stmt) {
        doc_count = sqlite3_column_int64(stmt, 0);
        total_size = sqlite3_column_int64(stmt, 1);
    });
    UTF8String sep = UTF8String(separator);
    DocumentStore store(sep);
    store.reserve(total_size);
    if (verbose_) std::cout << "Building Query" << std::endl;
    std::string query = buildQuery(
        table_name, filter_column, content_column, filter_value
    );
    int current = 0;
    if (verbose_) std::cout << "Adding Documents" << std::endl;
    executeQuery(query, [&store, &current, this, &doc_count](sqlite3_stmt* stmt) {
        const unsigned char* content = sqlite3_column_text(stmt, 0);
        const int64_t id = sqlite3_column_int64(stmt, 1);
        if (verbose_) {
            if (current % std::max((int)doc_count / 100, 100) == 0 || current == doc_count) {
                        float percentage = (float)current / doc_count * 100;
                        std::cout << "\rProcessing documents... " << current << "/" << doc_count
                                 << " (" << std::fixed << std::setprecision(1) << percentage << "%)"
                                 << std::flush;
                    }
        }
        std::string content_str(reinterpret_cast<const char*>(content));
        store.add_document(UTF8String(content_str), id);
        current++;
    });

    return store;
}

std::pair<bool, std::string> SQLiteHandler::validateTableAndColumns(
    const std::string& table_name,
    const std::vector<std::string>& columns
) {
    std::stringstream query;
    query << "PRAGMA table_info(" << sanitizeInput(table_name) << ")";

    bool table_exists = false;
    std::vector<std::string> existing_columns;

    executeQuery(query.str(), [&](sqlite3_stmt* stmt) {
        table_exists = true;
        const unsigned char* column = sqlite3_column_text(stmt, 1);
        existing_columns.emplace_back(reinterpret_cast<const char*>(column));
    });

    if (!table_exists) {
        return {false, table_name};
    }

    for (const auto& column : columns) {
        if (std::find(existing_columns.begin(), existing_columns.end(), column)
            == existing_columns.end()) {
            return {false, column};
        }
    }

    return {true, ""};
}

void SQLiteHandler::updateRow(
    const std::string& table_name,
    int64_t row_id,
    const std::string& column_name,
    const std::string& new_value
) {
    std::stringstream query;
    query << "UPDATE " << sanitizeInput(table_name)
          << " SET " << sanitizeInput(column_name) << " = '"
          << sanitizeInput(new_value) << "' WHERE rowid = " << row_id;

    executeQuery(query.str(), [](sqlite3_stmt*){});
}

template <typename Callback>
void SQLiteHandler::executeQuery(const std::string& query, Callback callback) {
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_connection_, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw SQLiteError("Failed to prepare SQL statement: " + query);
    }

    int rc = 0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        callback(stmt);
    }

    if (rc != SQLITE_DONE) {
        throw SQLiteError("Failed to execute SQL statement: " + query);
    }

    sqlite3_finalize(stmt);
}

std::string SQLiteHandler::sanitizeInput(const std::string& input) {
    std::string sanitized = input;
    for (size_t pos = sanitized.find('\'');
         pos != std::string::npos;
         pos = sanitized.find('\'', pos)) {
        sanitized.replace(pos, 1, "''");
        pos += 2;
    }
    return sanitized;
}

std::string SQLiteHandler::buildQuery(
    const std::string& table_name,
    const std::string& filter_column,
    const std::string& content_column,
    const std::string& filter_value
) {
    // Validate table and column names
    if (!isValidName(table_name)) {
        throw SQLiteError("Invalid table name: " + table_name);
    }
    if (!isValidName(filter_column)) {
        throw SQLiteError("Invalid column name: " + filter_column);
    }
    if (!isValidName(content_column)) {
        throw SQLiteError("Invalid column name: " + content_column);
    }

    std::stringstream query;
    query << "SELECT " << sanitizeInput(content_column)
          << ", rowid FROM " << sanitizeInput(table_name)
          << " WHERE " << sanitizeInput(filter_column)
          << " = '" << sanitizeInput(filter_value) << "'";
    return query.str();
}

} // namespace text_processing