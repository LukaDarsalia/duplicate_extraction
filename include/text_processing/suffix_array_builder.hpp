#ifndef TEXT_PROCESSING_SUFFIX_ARRAY_BUILDER_HPP
#define TEXT_PROCESSING_SUFFIX_ARRAY_BUILDER_HPP

#include <vector>
#include "text_processing/utf8_handler.hpp"

namespace text_processing {
    /**
     * @brief Abstract interface for suffix array construction
     *
     * This class defines the interface for building and accessing suffix arrays.
     * Different implementations can provide various construction algorithms
     * (naive, SA-IS, etc.) while maintaining the same interface.
     */
    class SuffixArrayBuilder {
    public:
        /**
         * @brief Types of available suffix array builders
         */
        enum class BuilderType {
            NAIVE, ///< Naive O(n*log(n)) implementation
            // Future implementations can be added here
            // SAIS,    ///< SA-IS algorithm implementation
            // KS,      ///< Kärkkäinen-Sanders algorithm implementation
        };

        /**
         * @brief Virtual destructor for proper cleanup
         */
        virtual ~SuffixArrayBuilder() = default;

        /**
         * @brief Build suffix array from UTF8String
         *
         * @param text Input text to build suffix array from
         * @return true if building was successful
         * @throw std::runtime_error if building fails
         */
        virtual bool build(const UTF8String &text) = 0;

        /**
         * @brief Get the constructed suffix array
         *
         * @return const reference to the suffix array vector
         * @throw std::runtime_error if array hasn't been built
         */
        [[nodiscard]] virtual const std::vector<size_t> &get_array() const = 0;

        /**
         * @brief Get the Longest Common Prefix (LCP) array
         *
         * LCP array contains the lengths of the longest common prefix between
         * consecutive suffixes in the suffix array.
         *
         * @return const reference to the LCP array
         * @throw std::runtime_error if array hasn't been built
         */
        [[nodiscard]] virtual const std::vector<size_t> &get_lcp_array() const = 0;

        /**
         * @brief Get original text the suffix array was built from
         *
         * @return const reference to the original text
         */
        [[nodiscard]] virtual const UTF8String &get_text() const = 0;

        /**
         * @brief Check if suffix array has been built
         *
         * @return true if suffix array is built and ready
         */
        virtual bool is_built() const = 0;

        /**
         * @brief Create a builder of specific type
         *
         * Factory method to create concrete implementations of the builder.
         * Each implementation should register its type in the enum and
         * provide creation logic in this method.
         *
         * @param type Type of suffix array builder to create
         * @return std::unique_ptr<SuffixArrayBuilder> Pointer to created builder
         * @throw std::invalid_argument if type is invalid
         */
        static std::unique_ptr<SuffixArrayBuilder> create(BuilderType type);
    };
} // namespace text_processing

#endif // TEXT_PROCESSING_SUFFIX_ARRAY_BUILDER_HPP
