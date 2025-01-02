#ifndef TEXT_PROCESSING_NAIVE_SUFFIX_BUILDER_HPP
#define TEXT_PROCESSING_NAIVE_SUFFIX_BUILDER_HPP

#include <vector>
#include "text_processing/suffix_array_builder.hpp"
#include "text_processing/utf8_handler.hpp"

namespace text_processing {

/**
 * @brief O(n*log(n)) implementation of suffix array construction using cyclic shifts
 *
 * This implementation uses the following approach:
 * 1. Sort single characters and assign equivalence classes
 * 2. For k=1..log(n):
 *    - Use previous sorting of length 2^(k-1) to sort length 2^k substrings
 *    - Update equivalence classes
 * 3. Build LCP array using Kasai's algorithm
 * 
 * Space complexity: O(n)
 * Time complexity: O(n*log(n))
 */
class NaiveSuffixBuilder : public SuffixArrayBuilder {
public:
    /**
     * @brief Default constructor
     */
    NaiveSuffixBuilder() = default;

    /**
     * @brief Build suffix array from UTF8String
     * 
     * @param text Input text to build suffix array from
     * @return true if building was successful
     * @throw std::runtime_error if building fails or text is empty
     */
    bool build(const UTF8String& text) override;

    /**
     * @brief Get the constructed suffix array
     * 
     * @return const reference to the suffix array vector
     * @throw std::runtime_error if array hasn't been built
     */
    [[nodiscard]] const std::vector<size_t>& get_array() const override;

    /**
     * @brief Get the Longest Common Prefix (LCP) array
     * 
     * @return const reference to the LCP array
     * @throw std::runtime_error if array hasn't been built
     */
    [[nodiscard]] const std::vector<size_t>& get_lcp_array() const override;

    /**
     * @brief Get original text the suffix array was built from
     * 
     * @return const reference to the original text
     */
    [[nodiscard]] const UTF8String& get_text() const override;

    /**
     * @brief Check if suffix array has been built
     * 
     * @return true if suffix array is built and ready
     */
    bool is_built() const override;

private:
    UTF8String text_;                    ///< Original text
    std::vector<size_t> suffix_array_;   ///< Constructed suffix array
    std::vector<size_t> lcp_array_;      ///< LCP array
    bool is_built_ = false;              ///< Construction state flag

    /**
     * @brief Initial sorting of single characters
     * 
     * Creates initial suffix array and equivalence classes based on first characters.
     * Uses counting sort for O(n) complexity.
     * 
     * @param p Permutation array (suffix array)
     * @param c Equivalence classes array
     * @return Number of distinct equivalence classes
     */
    size_t sort_characters(std::vector<size_t>& p, std::vector<size_t>& c) const;

    /**
     * @brief Sort cyclic substrings of length 2^k
     * 
     * Uses previous sorting of length 2^(k-1) to sort substrings of length 2^k.
     * 
     * @param k Current power (length = 2^k)
     * @param p Permutation array (suffix array)
     * @param c Equivalence classes array
     * @param classes Number of equivalence classes
     * @return New number of equivalence classes
     */
    size_t sort_doubled(size_t k, std::vector<size_t>& p, std::vector<size_t>& c, size_t classes) const;

    /**
     * @brief Build the LCP array using Kasai's algorithm
     * 
     * Constructs LCP array in O(n) time using the constructed suffix array.
     */
    void build_lcp_array();

    /**
     * @brief Create rank array from suffix array
     * 
     * Inverse of the suffix array - for each position in the text,
     * stores its position in the suffix array. Used in Kasai's algorithm.
     * 
     * @return vector of ranks
     */
    [[nodiscard]] std::vector<size_t> create_rank_array() const;

    /**
     * @brief Validate input text
     * 
     * @param text Text to validate
     * @throw std::invalid_argument if text is empty
     */
    static void validate_input(const UTF8String& text);
};

} // namespace text_processing

#endif // TEXT_PROCESSING_NAIVE_SUFFIX_BUILDER_HPP