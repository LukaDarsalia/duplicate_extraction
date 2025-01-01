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

// Test string comparison
TEST_F(UTF8StringTest, StringComparison) {
    UTF8String str1("აბგ");
    UTF8String str2("აბგ");
    UTF8String str3("აბდ");

    EXPECT_EQ(str1, str2);
    EXPECT_NE(str1, str3);
    EXPECT_LT(str1, str3);
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
    EXPECT_EQ(str3.length(), 4);
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