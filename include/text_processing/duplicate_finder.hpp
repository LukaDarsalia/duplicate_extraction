#ifndef TEXT_PROCESSING_DUPLICATE_FINDER_HPP
#define TEXT_PROCESSING_DUPLICATE_FINDER_HPP

#include <fstream>
#include <vector>
#include "data/document_store.hpp"
#include "text_processing/suffix_array_builder.hpp"
#include "data/duplicate_match.hpp"

namespace text_processing {
    /**
     * @brief Class for finding duplicate text between documents using suffix arrays
     */
    class DuplicateFinder {
    public:
        /**
         * @brief Constructor
         * @param builder_type Type of suffix array builder to use
         */
        explicit DuplicateFinder(
            SuffixArrayBuilder::BuilderType builder_type = SuffixArrayBuilder::BuilderType::NAIVE
        );

        /**
         * @brief Find duplicate text between documents
         *
         * @param store Document store containing the texts to analyze
         * @param min_length Minimum length of duplicate substring to report
         * @return std::vector<Match> Vector of found matches, sorted by length (descending)
         * @throw std::runtime_error if suffix array construction fails
         */
        std::vector<Match> find_duplicates(const DocumentStore &store, size_t min_length, bool verbose=false);

        static void save_matches_to_json(
            const std::vector<Match> &matches,
            const std::string &filename
        ) {
            std::ofstream out_file(filename);
            if (!out_file.is_open()) {
                throw std::runtime_error("Unable to open file: " + filename);
            }

            out_file << Match::to_json_array(matches);
            out_file.close();
        }

    private:
        std::unique_ptr<SuffixArrayBuilder> suffix_builder_;

        /**
         * @brief Process the LCP array to find duplicate substrings
         *
         * @param store Document store for position lookup
         * @param min_length Minimum length threshold
         * @return std::vector<Match> Vector of matches found
         */
        [[nodiscard]] std::vector<Match> process_matches(
            const DocumentStore &store,
            size_t min_length
        ) const;
    };
} // namespace text_processing

#endif // TEXT_PROCESSING_DUPLICATE_FINDER_HPP
