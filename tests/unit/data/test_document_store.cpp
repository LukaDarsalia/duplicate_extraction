#include <gtest/gtest.h>
#include "data/document_store.hpp"

using namespace text_processing;

class DocumentStoreTest : public ::testing::Test {
protected:
    void SetUp() override {
        store = std::make_unique<DocumentStore>();
    }

    void TearDown() override {
        store.reset();
    }

    // Helper to add sample documents
    void add_sample_documents() {
        store->add_document(UTF8String("Hello World"), 1);
        store->add_document(UTF8String("გამარჯობა"), 2);
        store->add_document(UTF8String("Testing 123"), 3);
    }

    std::unique_ptr<DocumentStore> store;
};

// Test adding documents
TEST_F(DocumentStoreTest, AddDocument) {
    EXPECT_TRUE(store->add_document(UTF8String("Test"), 1));
    EXPECT_TRUE(store->add_document(UTF8String("Another"), 2));
}

// Test finding document by position
TEST_F(DocumentStoreTest, FindDocumentId) {
    add_sample_documents();

    // First document
    auto doc1 = store->find_document_id(0);
    EXPECT_EQ(doc1.sql_id, 1);
    EXPECT_EQ(doc1.start_pos, 0);
    EXPECT_EQ(doc1.length, 11); // "Hello World" length

    auto doc2 = store->find_document_id(11);
    EXPECT_EQ(doc2.sql_id, 1);
    EXPECT_EQ(doc2.start_pos, 0);
    EXPECT_EQ(doc2.length, 11); // "გამარჯობა" length

    auto doc3 = store->find_document_id(12);
    EXPECT_EQ(doc3.sql_id, 2);
    EXPECT_EQ(doc3.start_pos, 12);
    EXPECT_EQ(doc3.length, 9); // "გამარჯობა" length

    auto doc4 = store->find_document_id(21);
    EXPECT_EQ(doc4.sql_id, 2);
    EXPECT_EQ(doc4.start_pos, 12);
    EXPECT_EQ(doc4.length, 9); // "Testing 123" length

    auto doc5 = store->find_document_id(22);
    EXPECT_EQ(doc5.sql_id, 3);
    EXPECT_EQ(doc5.start_pos, 22);
    EXPECT_EQ(doc5.length, 11); // "Testing 123" length

    // Invalid position
    EXPECT_THROW(store->find_document_id(999), std::out_of_range);
}

// Test concatenated text
TEST_F(DocumentStoreTest, ConcatenatedText) {
    add_sample_documents();
    UTF8String expected = UTF8String("Hello World") + UTF8String("$") +
                         UTF8String("გამარჯობა") + UTF8String("$") +
                         UTF8String("Testing 123") + UTF8String("$");
    EXPECT_EQ(store->get_concatenated_text(), expected);
}

// Test custom separator
TEST_F(DocumentStoreTest, CustomSeparator) {
    auto custom_store = std::make_unique<DocumentStore>(UTF8String("###"));
    custom_store->add_document(UTF8String("Doc1"), 1);
    custom_store->add_document(UTF8String("Doc2"), 2);

    UTF8String expected = UTF8String("Doc1") + UTF8String("###") + UTF8String("Doc2") + UTF8String("###");
    EXPECT_EQ(custom_store->get_concatenated_text(), expected);
}