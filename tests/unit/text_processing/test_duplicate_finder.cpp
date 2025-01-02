#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "text_processing/duplicate_finder.hpp"

using namespace text_processing;
using ::testing::ElementsAre;
using ::testing::UnorderedElementsAre;

class DuplicateFinderTest : public ::testing::Test {
protected:
    void SetUp() override {
        finder = std::make_unique<DuplicateFinder>();
        store = std::make_unique<DocumentStore>();
    }

    void TearDown() override {
        finder.reset();
        store.reset();
    }

    std::unique_ptr<DuplicateFinder> finder;
    std::unique_ptr<DocumentStore> store;

    // Helper to create a Match object
    static Match create_match(int64_t doc1_id, int64_t doc2_id, 
                            size_t start1, size_t start2, size_t len) {
        return Match{doc1_id, doc2_id, start1, start2, len};
    }
};

// Basic functionality tests
TEST_F(DuplicateFinderTest, EmptyStore) {
    auto matches = finder->find_duplicates(*store, 5);
    EXPECT_TRUE(matches.empty());
}

TEST_F(DuplicateFinderTest, SingleDocument) {
    store->add_document(UTF8String("Test document"), 1);
    auto matches = finder->find_duplicates(*store, 5);
    EXPECT_TRUE(matches.empty());
}

TEST_F(DuplicateFinderTest, NoMatches) {
    store->add_document(UTF8String("First document"), 1);
    store->add_document(UTF8String("Second text"), 2);
    auto matches = finder->find_duplicates(*store, 5);
    EXPECT_TRUE(matches.empty());
}

TEST_F(DuplicateFinderTest, SimpleMatch) {
    store->add_document(UTF8String("hello world"), 1);
    store->add_document(UTF8String("Say hello world"), 2);
    
    auto matches = finder->find_duplicates(*store, 5);
    EXPECT_THAT(matches, ElementsAre(
        create_match(1, 2, 0, 4, 11)  // "hello world"
    ));
}

TEST_F(DuplicateFinderTest, MultipleMatches) {
    store->add_document(UTF8String("The quick brown fox"), 1);
    store->add_document(UTF8String("The slow brown cat"), 2);
    
    auto matches = finder->find_duplicates(*store, 4);
    EXPECT_THAT(matches, UnorderedElementsAre(
        create_match(1, 2, 0, 0, 4),   // "The "
        create_match(1, 2, 9, 9, 7)  // " brown "
    ));
}


// UTF-8 specific tests
TEST_F(DuplicateFinderTest, UTF8Match) {
    store->add_document(UTF8String("გამარჯობა მსოფლიო"), 1);
    store->add_document(UTF8String("გამარჯობა კარგო"), 2);
    
    auto matches = finder->find_duplicates(*store, 5);
    EXPECT_THAT(matches, ElementsAre(
        create_match(1, 2, 0, 0, 10)  // "გამარჯობა "
    ));
}

// Length threshold tests
TEST_F(DuplicateFinderTest, ThresholdFiltering) {
    store->add_document(UTF8String("The quick brown fox"), 1);
    store->add_document(UTF8String("The slow brown cat"), 2);
    
    // With threshold 5, should only get "brown"
    auto matches5 = finder->find_duplicates(*store, 5);
    EXPECT_THAT(matches5, ElementsAre(
        create_match(1, 2, 9, 9, 7)  // " brown "
    ));
    
    // With threshold 3, should get both "The" and "brown"
    auto matches3 = finder->find_duplicates(*store, 3);
    EXPECT_THAT(matches3, UnorderedElementsAre(
        create_match(1, 2, 0, 0, 4),   // "The "
        create_match(1, 2, 9, 9, 7)  // " brown "
    ));
}

// Edge cases
TEST_F(DuplicateFinderTest, ZeroThreshold) {
    store->add_document(UTF8String("test"), 1);
    store->add_document(UTF8String("test"), 2);
    
    auto matches = finder->find_duplicates(*store, 0);
    EXPECT_THAT(matches, ElementsAre(
        create_match(1, 2, 0, 0, 4)  // "test"
    ));
}

TEST_F(DuplicateFinderTest, LargeThreshold) {
    store->add_document(UTF8String("short text"), 1);
    store->add_document(UTF8String("short text"), 2);
    
    auto matches = finder->find_duplicates(*store, 100);
    EXPECT_TRUE(matches.empty());
}