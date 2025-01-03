#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <filesystem>
#include "sql/sql_handler.hpp"

using namespace text_processing;
using ::testing::Contains;
namespace fs = std::filesystem;

class SQLiteHandlerTest : public ::testing::Test {
protected:
    const std::string ORIGINAL_DB_PATH = "test_documents.db";
    const std::string PREFIXED_DB_PATH = "original_test_documents.db";
    const std::string TEST_DB_PATH = "test_documents_temp.db";

    void SetUp() override {
        // Copy the original .db file to a prefixed backup if it doesn't already exist
        if (!fs::exists(PREFIXED_DB_PATH)) {
            if (fs::exists(ORIGINAL_DB_PATH)) {
                fs::copy(ORIGINAL_DB_PATH, PREFIXED_DB_PATH);
            } else {
                FAIL() << "Original database file not found at: " << ORIGINAL_DB_PATH;
            }
        }

        // Copy the prefixed backup file to the test-specific .db file
        if (fs::exists(TEST_DB_PATH)) {
            fs::remove(TEST_DB_PATH);
        }
        fs::copy(PREFIXED_DB_PATH, TEST_DB_PATH);

        // Initialize the handler with the test-specific database file
        handler = std::make_unique<SQLiteHandler>(TEST_DB_PATH);
    }

    void TearDown() override {
        handler.reset();

        // Restore the test-specific .db file from the prefixed backup
        if (fs::exists(TEST_DB_PATH)) {
            fs::remove(TEST_DB_PATH);
        }
        fs::copy(PREFIXED_DB_PATH, TEST_DB_PATH);
    }


    std::unique_ptr<SQLiteHandler> handler;
};

// Test that data was properly converted
TEST_F(SQLiteHandlerTest, CreateDocumentStore) {
    auto store = handler->createDocumentStore(
        "data_table",
        "domain",
        "content",
        "domain1.com"
    );

    EXPECT_EQ(store.get_concatenated_text().str(),
              "First document content$Second document from domain1$Third document from domain1$");
}

TEST_F(SQLiteHandlerTest, FilterByOtherColumn) {
    auto store = handler->createDocumentStore(
        "data_table",
        "category",
        "content",
        "blog"
    );

    EXPECT_EQ(store.get_concatenated_text().str(),
              "Document from domain2$Third document from domain1$");
}

TEST_F(SQLiteHandlerTest, UTF8ContentFromParquet) {
    auto store = handler->createDocumentStore(
        "data_table",
        "domain",
        "content",
        "domain3.com"
    );

    EXPECT_EQ(store.get_concatenated_text().str(),
              "გამარჯობა from domain3$");
}

TEST_F(SQLiteHandlerTest, EmptyFilterResult) {
    auto store = handler->createDocumentStore(
        "data_table",
        "domain",
        "content",
        "nonexistent.com"
    );

    EXPECT_EQ(store.get_concatenated_text().str(), "");
}

// Verify table structure
TEST_F(SQLiteHandlerTest, ValidTableAndColumns) {
    auto [valid, missing] = handler->validateTableAndColumns(
        "data_table",
        {"domain", "content"}
    );
    EXPECT_TRUE(valid);
    EXPECT_EQ(missing, "");
}

TEST_F(SQLiteHandlerTest, InvalidTableName) {
    auto [valid, missing] = handler->validateTableAndColumns(
        "nonexistent_table",
        {"domain", "content"}
    );
    EXPECT_FALSE(valid);
    EXPECT_EQ(missing, "nonexistent_table");
}

TEST_F(SQLiteHandlerTest, InvalidColumnName) {
    auto [valid, missing] = handler->validateTableAndColumns(
        "data_table",
        {"domain", "nonexistent_column"}
    );
    EXPECT_FALSE(valid);
    EXPECT_EQ(missing, "nonexistent_column");
}

// Error cases
TEST_F(SQLiteHandlerTest, MalformedTableName) {
    EXPECT_THROW(
        handler->createDocumentStore(
            "'; DROP TABLE data_table; --",
            "domain",
            "content",
            "domain1.com"
        ),
        SQLiteError
    );
}

TEST_F(SQLiteHandlerTest, MalformedColumnNames) {
    EXPECT_THROW(
        handler->createDocumentStore(
            "data_table",
            "'; DROP TABLE data_table; --",
            "content",
            "domain1.com"
        ),
        SQLiteError
    );
}

// Move semantics tests
TEST_F(SQLiteHandlerTest, MoveConstructor) {
    SQLiteHandler original(TEST_DB_PATH);
    SQLiteHandler moved(std::move(original));

    EXPECT_NO_THROW(
        moved.createDocumentStore(
            "data_table",
            "domain",
            "content",
            "domain1.com"
        )
    );
}

TEST_F(SQLiteHandlerTest, MoveAssignment) {
    SQLiteHandler original(TEST_DB_PATH);
    SQLiteHandler other(":memory:");
    other = std::move(original);

    EXPECT_NO_THROW(
        other.createDocumentStore(
            "data_table",
            "domain",
            "content",
            "domain1.com"
        )
    );
}

TEST_F(SQLiteHandlerTest, UpdateRow) {
    // Assuming we have a row with id 1 and content "Original content"
    handler->updateRow(
        "data_table",
        1,
        "content",
        "Updated content"
    );

    auto store = handler->createDocumentStore(
        "data_table",
        "domain",
        "content",
        "domain1.com"
    );
    std::string str = store.get_concatenated_text().str(), target = "Updated content";
    EXPECT_TRUE(str.find(target) != std::string::npos);
}