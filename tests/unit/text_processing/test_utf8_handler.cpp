#include <gtest/gtest.h>
#include "text_processing/utf8_handler.hpp"

using namespace text_processing;

// For tests that don't need the fixture
TEST(UTF8BasicTest, BasicSetup) {
    EXPECT_TRUE(true);
}

class UTF8StringTest : public ::testing::Test {
protected:
    // Test strings in different scripts
    const std::string georgian_text = "გამარჯობა";      // Georgian
    const std::string russian_text = "Привет";         // Russian
    const std::string chinese_text = "你好世界";        // Chinese
    const std::string mixed_text = "Hello გამარჯობა 你好"; // Mixed scripts
    const std::string emoji_text = "Hello 👋 World 🌍"; // With emojis
};

// Test construction and basic properties
TEST_F(UTF8StringTest, Construction) {
    ASSERT_NO_THROW({
        UTF8String str(georgian_text);
        EXPECT_EQ(str.length(), 9);  // გ-ა-მ-ა-რ-ჯ-ო-ბ-ა

        UTF8String str2(russian_text);
        EXPECT_EQ(str2.length(), 6);  // П-р-и-в-е-т

        UTF8String str3(chinese_text);
        EXPECT_EQ(str3.length(), 4);  // 你-好-世-界
    });
}

// Test invalid UTF-8 handling
TEST_F(UTF8StringTest, InvalidUTF8) {
    // Create some invalid UTF-8 sequences
    std::string invalid1{static_cast<char>(0xFF)};  // Single invalid byte
    std::string invalid2 = "Hello" + std::string{static_cast<char>(0xFF)};  // Invalid byte in string
    std::string truncated = georgian_text.substr(0, georgian_text.length() - 1);  // Truncated multi-byte

    EXPECT_THROW({UTF8String s1(invalid1);}, UTF8Error);
    EXPECT_THROW({UTF8String s2(invalid2);}, UTF8Error);
    EXPECT_THROW({UTF8String s3(truncated);}, UTF8Error);
}

// Test character access
TEST_F(UTF8StringTest, CharacterAccess) {
    UTF8String str(georgian_text);

    EXPECT_EQ(str[0].str(), "გ");
    EXPECT_EQ(str[1].str(), "ა");
    EXPECT_EQ(str[2].str(), "მ");
    EXPECT_EQ(str[7].str(), "ბ");

    // Test out of range access
    EXPECT_THROW(str[9], std::out_of_range);
}

// Test string iteration
TEST_F(UTF8StringTest, Iteration) {
    UTF8String str(georgian_text);
    std::vector<std::string> expected = {"გ", "ა", "მ", "ა", "რ", "ჯ", "ო", "ბ", "ა"};

    size_t index = 0;
    for (const auto& ch : str) {
        ASSERT_LT(index, expected.size());
        EXPECT_EQ(ch.str(), expected[index]);
        index++;
    }
    EXPECT_EQ(index, expected.size());
}

// Test substring operations
TEST_F(UTF8StringTest, Substring) {
    UTF8String str(mixed_text);

    // Test various substring operations
    EXPECT_EQ(str.substr(0, 5).str(), "Hello");
    EXPECT_EQ(str.substr(6, 9).str(), "გამარჯობა");

    // Test invalid substring parameters
    EXPECT_THROW({str.substr(str.length() + 1, 1);}, std::out_of_range);
    EXPECT_THROW({str.substr(0, str.length() + 1);}, std::out_of_range);
}

// Test character comparison
TEST_F(UTF8StringTest, CharacterComparison) {
    UTF8String str1("აბგ");
    UTF8String str2("აბგ");
    UTF8String str3("აბდ");

    // Test character equality
    EXPECT_EQ(str1[0], str2[0]);
    EXPECT_EQ(str1[1], str2[1]);

    // Test character inequality
    EXPECT_NE(str1[2], str3[2]);

    // Test character ordering
    EXPECT_LT(str1[2], str3[2]);  // გ < დ
}

// Test character comparison
TEST_F(UTF8StringTest, CharacterComparisonBetweenCodes) {
    UTF8String str1("აბგ#\x01");

    // Test character ordering
    EXPECT_LT(str1[3], str1[0]);  // # < გ
    EXPECT_LT(str1[4], str1[0]);  // \x01 < გ
    EXPECT_EQ(str1.length(), 5);
}


// Test string comparison
TEST_F(UTF8StringTest, StringComparison) {
    UTF8String str1("აბგ");
    UTF8String str2("აბგ");
    UTF8String str3("აბდ");

    EXPECT_EQ(str1, str2);
    EXPECT_NE(str1, str3);
    EXPECT_LT(str1, str3);
}

// Test string comparison
TEST_F(UTF8StringTest, StringComparisonBetweenCodes) {
    UTF8String str1("\x01");
    UTF8String str2("ა");

    EXPECT_LT(str1, str2);
}

// Test emoji handling
TEST_F(UTF8StringTest, EmojiHandling) {
    UTF8String str(emoji_text);

    std::vector<std::string> expected = {"H", "e", "l", "l", "o", " ", "👋", " ",
                                       "W", "o", "r", "l", "d", " ", "🌍"};

    EXPECT_EQ(str.length(), expected.size());

    size_t index = 0;
    for (const auto& ch : str) {
        ASSERT_LT(index, expected.size());
        EXPECT_EQ(ch.str(), expected[index]);
        index++;
    }
}

// Test empty string handling
TEST_F(UTF8StringTest, EmptyString) {
    UTF8String str("");

    EXPECT_EQ(str.length(), 0);
    EXPECT_EQ(str.begin(), str.end());
    EXPECT_THROW(str[0], std::out_of_range);
}

// Test concatenation
TEST_F(UTF8StringTest, Concatenation) {
    UTF8String str1("გა");
    UTF8String str2("მარ");
    UTF8String str3 = str1 + str2;

    EXPECT_EQ(str3.str(), "გამარ");
    EXPECT_EQ(str3.length(), 5);
}

// Basic concatenation tests
TEST_F(UTF8StringTest, BasicAppend) {
    UTF8String str1("Hello");
    const UTF8String str2(" World");

    str1 += str2;
    EXPECT_EQ(str1.str(), "Hello World");
    EXPECT_EQ(str1.length(), 11);
}

// Test empty string handling
TEST_F(UTF8StringTest, EmptyStrings) {
    UTF8String empty;
    UTF8String str("Hello");

    // Append to empty string
    empty += str;
    EXPECT_EQ(empty.str(), "Hello");
    EXPECT_EQ(empty.length(), 5);

    // Append empty string
    UTF8String str2("World");
    str2 += UTF8String();
    EXPECT_EQ(str2.str(), "World");
    EXPECT_EQ(str2.length(), 5);

    // Append empty to empty
    UTF8String empty1, empty2;
    empty1 += empty2;
    EXPECT_EQ(empty1.str(), "");
    EXPECT_EQ(empty1.length(), 0);
}

// Test UTF-8 character handling
TEST_F(UTF8StringTest, UTF8Characters) {
    UTF8String str1("გამარჯობა");
    UTF8String str2(" მსოფლიო");

    str1 += str2;
    EXPECT_EQ(str1.length(), 17);  // Count of actual UTF-8 characters

    // Verify character access after concatenation
    EXPECT_EQ(str1[0].str(), "გ");
    EXPECT_EQ(str1[8].str(), "ა");
    EXPECT_EQ(str1[9].str(), " ");
}

// Test chained operations
TEST_F(UTF8StringTest, ChainedOperations) {
    UTF8String str1("One");
    UTF8String str2(" Two");
    UTF8String str3(" Three");

    // Test chaining += operations
    (str1 += str2) += str3;

    EXPECT_EQ(str1.str(), "One Two Three");
    EXPECT_EQ(str1.length(), 13);
}

// Test mixed character sets
TEST_F(UTF8StringTest, MixedCharacterSets) {
    UTF8String str1("Hello გამარჯობა");
    UTF8String str2(" 你好 World");

    str1 += str2;

    // Verify total length
    size_t expected_length = 5 + 1 + 9 + 1 + 2 + 1 + 5; // Hello + Georgian + space + Chinese + space + World
    EXPECT_EQ(str1.length(), expected_length);

    // Verify character preservation
    EXPECT_EQ(str1[6].str(), "გ");
}

// Test large string handling
TEST_F(UTF8StringTest, LargeStrings) {
    // Create a large string with repeated content
    std::string large_content(1000, 'a');
    UTF8String large_str1(large_content);
    UTF8String large_str2(large_content);

    large_str1 += large_str2;
    EXPECT_EQ(large_str1.length(), 2000);
    EXPECT_EQ(large_str1[0].str(), "a");
    EXPECT_EQ(large_str1[1999].str(), "a");
}

// Test appending std::string
TEST_F(UTF8StringTest, StdStringAppend) {
    UTF8String utf8_str("Hello");
    std::string std_str(" World");

    utf8_str += std_str;
    EXPECT_EQ(utf8_str.str(), "Hello World");
    EXPECT_EQ(utf8_str.length(), 11);
}


// Test self-append
TEST_F(UTF8StringTest, SelfAppend) {
    UTF8String str("Test");
    str += str;
    EXPECT_EQ(str.str(), "TestTest");
    EXPECT_EQ(str.length(), 8);
}

// Test performance with multiple appends
TEST_F(UTF8StringTest, MultipleAppends) {
    UTF8String str;
    const UTF8String append_str("test");

    // Perform multiple appends
    for(int i = 0; i < 1000; ++i) {
        str += append_str;
    }

    EXPECT_EQ(str.length(), 4000);  // 4 chars * 1000
}

// Test for correct handling of string boundaries
TEST_F(UTF8StringTest, StringBoundaries) {
    UTF8String str("აბგ");
    EXPECT_EQ(str.length(), 3);

    // Test that we can access the last character
    EXPECT_NO_THROW(str[2]);
    // Test that accessing past the end throws
    EXPECT_THROW(str[3], std::out_of_range);
}