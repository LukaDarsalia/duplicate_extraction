#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "text_processing/duplicate_finder.hpp"

using namespace text_processing;
using ::testing::ElementsAre;
using ::testing::UnorderedElementsAre;


// This was tested for longest common substring problem between two strings on codeforces and passed everything!
// https://codeforces.com/edu/course/2/lesson/2/5/practice/contest/269656/submission/299411160
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
        create_match(1, 2, 9, 8, 7)  // " brown "
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

// UTF-8 specific tests multiple
TEST_F(DuplicateFinderTest, UTF8MatchMultiple) {
    store->add_document(UTF8String("გამარჯობა მსოფლიო"), 1);
    store->add_document(UTF8String("გამარჯობა კარგო"), 2);
    store->add_document(UTF8String("ჩემო კარგო"), 3);
    store->add_document(UTF8String("მსოფლიო ულამაზესია!"), 4);


    auto matches = finder->find_duplicates(*store, 5);
    EXPECT_THAT(matches, UnorderedElementsAre(
        create_match(1, 2, 0, 0, 10), // "გამარჯობა "
        create_match(2, 3, 9, 4, 6),  // " კარგო"
        create_match(1, 4, 10, 0, 7)  // "მსოფლიო"
    ));
}

// Length threshold tests
TEST_F(DuplicateFinderTest, ThresholdFiltering) {
    store->add_document(UTF8String("The quick brown fox"), 1);
    store->add_document(UTF8String("The slow brown cat"), 2);
    
    // With threshold 5, should only get "brown"
    auto matches5 = finder->find_duplicates(*store, 5);
    EXPECT_THAT(matches5, ElementsAre(
        create_match(1, 2, 9, 8, 7)  // " brown "
    ));
    
    // With threshold 3, should get same
    auto matches3 = finder->find_duplicates(*store, 3);
    EXPECT_THAT(matches3, UnorderedElementsAre(
        create_match(1, 2, 9, 8, 7)  // " brown "
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

TEST_F(DuplicateFinderTest, SaveMatchesToJson) {
    store->add_document(UTF8String("Hello World"), 1);
    store->add_document(UTF8String("Say hello world"), 2);

    auto matches = finder->find_duplicates(*store, 5);

    // Create a temporary file
    const std::string temp_file = "test_matches.json";

    // Save matches
    EXPECT_NO_THROW({
        DuplicateFinder::save_matches_to_json(matches, temp_file);
    });

    // Read the file back and validate
    std::ifstream in_file(temp_file);
    ASSERT_TRUE(in_file.is_open());

    std::string content((std::istreambuf_iterator<char>(in_file)),
                        std::istreambuf_iterator<char>());

    // Do some basic validation using manual string checks
    EXPECT_FALSE(content.empty());
    EXPECT_EQ(content[0], '[');
    EXPECT_EQ(content[1], '{');
    EXPECT_EQ(content[content.length() - 1], ']');
    EXPECT_EQ(content[content.length() - 2], '}');

    // Clean up
    std::remove(temp_file.c_str());
}