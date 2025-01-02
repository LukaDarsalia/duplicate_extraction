#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "text_processing/naive_suffix_builder.hpp"
#include "text_processing/suffix_array_builder.hpp"

using namespace text_processing;
using ::testing::ElementsAre;

class NaiveSuffixBuilderTest : public ::testing::Test {
protected:
    void SetUp() override {
        builder = std::make_unique<NaiveSuffixBuilder>();
    }

    void TearDown() override {
        builder.reset();
    }

    // Helper function to build and verify
    bool buildAndVerify(const std::string& input, const std::vector<size_t>& expected_sa) {
        UTF8String text(input);
        if (!builder->build(text)) return false;
        const auto& sa = builder->get_array();
        return sa == expected_sa;
    }

    // Helper function to verify LCP array
    bool verifyLCP(const std::vector<size_t>& expected_lcp) {
        const auto& lcp = builder->get_lcp_array();
        return lcp == expected_lcp;
    }

    std::unique_ptr<NaiveSuffixBuilder> builder;
};

// Basic Test Cases
TEST_F(NaiveSuffixBuilderTest, EmptyString) {
    EXPECT_THROW(builder->build(UTF8String("")), std::runtime_error);
    EXPECT_FALSE(builder->is_built());
}

TEST_F(NaiveSuffixBuilderTest, SingleCharacter) {
    EXPECT_TRUE(buildAndVerify("a$", {1, 0}));
    EXPECT_TRUE(builder->is_built());
}

TEST_F(NaiveSuffixBuilderTest, SimpleUniqueChars) {
    EXPECT_TRUE(buildAndVerify("abc$", {3, 0, 1, 2}));
}

TEST_F(NaiveSuffixBuilderTest, RepeatingChars) {
    EXPECT_TRUE(buildAndVerify("aaa$", {3, 2, 1, 0}));
}

TEST_F(NaiveSuffixBuilderTest, BasicPattern) {
    EXPECT_TRUE(buildAndVerify("abab$", {4, 2, 0, 3, 1}));
}

// UTF-8 Test Cases
TEST_F(NaiveSuffixBuilderTest, GeorgianText) {
    EXPECT_TRUE(buildAndVerify("აბგ$", {3, 0, 1, 2}));
}

TEST_F(NaiveSuffixBuilderTest, MixedASCIIAndUTF8) {
    EXPECT_TRUE(builder->build(UTF8String("a და$")));
    EXPECT_TRUE(builder->is_built());
}

TEST_F(NaiveSuffixBuilderTest, EmojiText) {
    EXPECT_TRUE(builder->build(UTF8String("👋🌍$")));
    EXPECT_TRUE(builder->is_built());
}

// Classic Suffix Array Test
TEST_F(NaiveSuffixBuilderTest, BananaTest) {
    EXPECT_TRUE(buildAndVerify("banana$", {6, 5, 3, 1, 0, 4, 2}));
}

// LCP Array Tests
TEST_F(NaiveSuffixBuilderTest, SimpleLCPTest) {
    ASSERT_TRUE(builder->build(UTF8String("abcab$")));
    EXPECT_TRUE(verifyLCP({0, 2, 0, 1, 0}));
}

TEST_F(NaiveSuffixBuilderTest, RepeatingPatternLCP) {
    ASSERT_TRUE(builder->build(UTF8String("aaaa$")));
    EXPECT_TRUE(verifyLCP({0, 1, 2, 3}));
}

TEST_F(NaiveSuffixBuilderTest, SpecialCharacters) {
    EXPECT_TRUE(builder->build(UTF8String("!@#$%^&*()")));
}

// Interface Tests via Factory
TEST_F(NaiveSuffixBuilderTest, FactoryCreationAndUsage) {
    auto factory_builder = SuffixArrayBuilder::create(SuffixArrayBuilder::BuilderType::NAIVE);
    ASSERT_NE(factory_builder, nullptr);
    
    UTF8String text("test$");
    EXPECT_TRUE(factory_builder->build(text));
    EXPECT_TRUE(factory_builder->is_built());
    
    const auto& sa = factory_builder->get_array();
    EXPECT_EQ(sa.size(), 5);
}

// Builder State Tests
TEST_F(NaiveSuffixBuilderTest, BuilderStateTransitions) {
    EXPECT_FALSE(builder->is_built());
    
    UTF8String text("test$");
    EXPECT_TRUE(builder->build(text));
    EXPECT_TRUE(builder->is_built());
    
    // Test array access before and after build
    EXPECT_THROW(builder->get_array(), std::runtime_error);
    EXPECT_TRUE(builder->build(text));
    EXPECT_NO_THROW(builder->get_array());
}

// Property Verification Tests
TEST_F(NaiveSuffixBuilderTest, SuffixArrayProperties) {
    UTF8String text("banana$");
    ASSERT_TRUE(builder->build(text));
    
    const auto& sa = builder->get_array();
    
    // Property 1: Length check
    EXPECT_EQ(sa.size(), text.length());
    
    // Property 2: Valid indices
    for (size_t index : sa) {
        EXPECT_LT(index, text.length());
    }
    
    // Property 3: Uniqueness of indices
    std::vector<size_t> sorted_sa = sa;
    std::sort(sorted_sa.begin(), sorted_sa.end());
    for (size_t i = 0; i < sorted_sa.size(); ++i) {
        EXPECT_EQ(sorted_sa[i], i);
    }
    
    // Property 4: Sorted suffixes
    for (size_t i = 1; i < sa.size(); ++i) {
        UTF8String prev = text.substr(sa[i-1], text.length() - sa[i-1]);
        UTF8String curr = text.substr(sa[i], text.length() - sa[i]);
        EXPECT_LT(prev, curr) << "Suffixes not properly sorted at position " << i;
    }
}

// LCP Array Property Tests
TEST_F(NaiveSuffixBuilderTest, LCPArrayProperties) {
    UTF8String text("abcabc$");
    ASSERT_TRUE(builder->build(text));
    
    const auto& lcp = builder->get_lcp_array();
    const auto& sa = builder->get_array();
    
    // Property 1: Length check
    EXPECT_EQ(lcp.size(), text.length() - 1);
    
    // Property 2: LCP values cannot be larger than remaining string length
    for (size_t i = 0; i < lcp.size(); ++i) {
        size_t max_possible_lcp = text.length() - std::max(sa[i], sa[i+1]);
        EXPECT_LE(lcp[i], max_possible_lcp);
    }
}