#include "text_processing/naive_suffix_builder.hpp"
#include <stdexcept>
#include <map>

namespace text_processing {

bool NaiveSuffixBuilder::build(const UTF8String& text) {
    try {
        validate_input(text);
        text_ = text;
        is_built_ = false;

        // Initialize vectors for sorting and equivalence classes
        std::vector<size_t> p(text_.length());
        std::vector<size_t> c(text_.length());

        // Initial sorting of single characters
        size_t classes = sort_characters(p, c);

        // Main loop - sort by powers of 2
        size_t len = 1;
        while (len < text_.length()) {
            classes = sort_doubled(len, p, c, classes);
            len *= 2;
        }

        // Store final suffix array and build LCP array
        suffix_array_ = std::move(p);
        build_lcp_array();
        is_built_ = true;
        return true;
    } catch (const std::exception& e) {
        is_built_ = false;
        throw std::runtime_error(std::string("Failed to build suffix array: ") + e.what());
    }
}

size_t NaiveSuffixBuilder::sort_characters(std::vector<size_t>& p, std::vector<size_t>& c) const {
    const size_t n = text_.length();

    // Map each unique character to an index
    std::map<UTF8String::Character, size_t> char_index;
    for (size_t i = 0; i < n; i++) {
        char_index[text_[i]] = 0;  // Just insert to collect unique characters
    }

    // Assign indices to characters
    size_t idx = 0;
    for (auto& pair : char_index) {
        pair.second = idx++;
    }

    // Count characters using their indices
    std::vector<size_t> cnt(char_index.size(), 0);
    for (size_t i = 0; i < n; i++) {
        cnt[char_index[text_[i]]]++;
    }

    for (size_t i = 1; i < char_index.size(); i++) {
        cnt[i] += cnt[i-1];
    }

    for (size_t i = 0; i < n; i++) {
        p[--cnt[char_index[text_[i]]]] = i;
    }

    // Assign equivalence classes
    c[p[0]] = 0;
    size_t classes = 1;

    for (size_t i = 1; i < n; i++) {
        if (text_[p[i]] != text_[p[i-1]]) {
            classes++;
        }
        c[p[i]] = classes - 1;
    }

    return classes;
}

size_t NaiveSuffixBuilder::sort_doubled(const size_t k, std::vector<size_t>& p,
                                        std::vector<size_t>& c, size_t classes) const {
    const size_t n = text_.length();
    std::vector<size_t> cnt(classes, 0);
    std::vector<size_t> pn(n);
    std::vector<size_t> cn(n);
    
    // Sort by second element
    for (size_t i = 0; i < n; i++) {
        pn[i] = (p[i] + n - k) % n;
    }
    
    // Count sort by first element
    for (size_t i = 0; i < n; i++) {
        cnt[c[pn[i]]]++;
    }
    
    for (size_t i = 1; i < classes; i++) {
        cnt[i] += cnt[i-1];
    }
    
    for (size_t i = n; i > 0; i--) {
        p[--cnt[c[pn[i-1]]]] = pn[i-1];
    }
    
    // Update equivalence classes
    cn[p[0]] = 0;
    classes = 1;
    
    for (size_t i = 1; i < n; i++) {
        std::pair<size_t, size_t> cur = {c[p[i]], c[(p[i] + k) % n]};
        std::pair<size_t, size_t> prev = {c[p[i-1]], c[(p[i-1] + k) % n]};
        
        if (cur != prev) {
            classes++;
        }
        cn[p[i]] = classes - 1;
    }
    
    c = std::move(cn);
    return classes;
}

void NaiveSuffixBuilder::build_lcp_array() {
    const size_t n = text_.length();
    std::vector<size_t> rank = create_rank_array();
    lcp_array_.resize(n - 1);
    
    // Kasai's algorithm
    size_t k = 0;
    for (size_t i = 0; i < n; i++) {
        if (rank[i] == n - 1) {
            k = 0;
            continue;
        }
        
        size_t j = suffix_array_[rank[i] + 1];
        while (i + k < n && j + k < n && 
               text_[i + k] == text_[j + k]) {
            k++;
        }
        
        lcp_array_[rank[i]] = k;
        if (k > 0) k--;
    }
}

std::vector<size_t> NaiveSuffixBuilder::create_rank_array() const {
    std::vector<size_t> rank(text_.length());
    for (size_t i = 0; i < text_.length(); i++) {
        rank[suffix_array_[i]] = i;
    }
    return rank;
}

void NaiveSuffixBuilder::validate_input(const UTF8String& text) {
    if (text.length() == 0) {
        throw std::runtime_error("Empty string provided");
    }
}

const std::vector<size_t>& NaiveSuffixBuilder::get_array() const {
    if (!is_built_) {
        throw std::runtime_error("Suffix array not built");
    }
    return suffix_array_;
}

const std::vector<size_t>& NaiveSuffixBuilder::get_lcp_array() const {
    if (!is_built_) {
        throw std::runtime_error("LCP array not built");
    }
    return lcp_array_;
}

const UTF8String& NaiveSuffixBuilder::get_text() const {
    return text_;
}

bool NaiveSuffixBuilder::is_built() const {
    return is_built_;
}

} // namespace text_processing