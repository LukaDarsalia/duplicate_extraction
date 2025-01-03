#include "text_processing/duplicate_finder.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <map>


namespace text_processing {
    DuplicateFinder::DuplicateFinder(SuffixArrayBuilder::BuilderType builder_type)
        : suffix_builder_(SuffixArrayBuilder::create(builder_type)) {
    }

    std::vector<Match> DuplicateFinder::find_duplicates(
        const DocumentStore &store,
        size_t min_length,
        bool verbose
    ) {
        if (verbose) std::cout << "Returns concatenated text" << std::endl;
        // Get concatenated text from store
        const auto &text = store.get_concatenated_text();
        if (text.length() == 0) {
            return {};
        }
        if (verbose) std::cout << "Starts Building Suffix Array" << std::endl;
        // Build suffix array and LCP array
        if (!suffix_builder_->build(text)) {
            throw std::runtime_error("Failed to build suffix array");
        }
        if (verbose) std::cout << "Starts Finding Matches" << std::endl;
        return process_matches(store, min_length);
    }

    std::vector<Match> DuplicateFinder::process_matches(
        const DocumentStore &store,
        size_t min_length
    ) const {
        const auto &suffix_array = suffix_builder_->get_array();
        const auto &lcp_array = suffix_builder_->get_lcp_array();

        // Map to store longest match for each document pair
        // Key: a pair of doc IDs (smaller ID first), Value: best match found
        std::map<std::pair<int64_t, int64_t>, Match> best_matches;

        // Process all adjacent positions in suffix array
        for (size_t i = 0; i < lcp_array.size(); ++i) {
            try {
                // Get documents for adjacent positions in suffix array
                auto doc1 = store.find_document_id(suffix_array[i]);
                auto doc2 = store.find_document_id(suffix_array[i + 1]);

                // Skip if same document
                if (doc1.sql_id == doc2.sql_id) {
                    continue;
                }

                // Calculate relative positions within documents
                size_t pos1 = suffix_array[i] - doc1.start_pos;
                size_t pos2 = suffix_array[i + 1] - doc2.start_pos;

                // Adjust length if it crosses document boundaries
                size_t max_len1 = doc1.length - pos1;
                size_t max_len2 = doc2.length - pos2;
                size_t max_possible_length = std::min(max_len1, max_len2);
                size_t actual_length = std::min(lcp_array[i], max_possible_length);

                // Skip if too short
                if (actual_length < min_length) {
                    continue;
                }

                // Create document pair key (smaller ID first)
                auto doc_pair = std::make_pair(
                    std::min(doc1.sql_id, doc2.sql_id),
                    std::max(doc1.sql_id, doc2.sql_id)
                );

                // Create match (ensure doc1 is the one with smaller ID)
                Match match;
                if (doc1.sql_id < doc2.sql_id) {
                    match = Match{doc1.sql_id, doc2.sql_id, pos1, pos2, actual_length};
                } else {
                    match = Match{doc2.sql_id, doc1.sql_id, pos2, pos1, actual_length};
                }

                // Update best match for this document pair if longer
                auto it = best_matches.find(doc_pair);
                if (it == best_matches.end() || match.length > it->second.length) {
                    best_matches[doc_pair] = match;
                }
            } catch (const std::out_of_range &) {
                // Skip positions that fall in document separators
                continue;
            }
        }

        // Convert map to vector of matches
        std::vector<Match> result;
        result.reserve(best_matches.size());
        for (const auto &pair: best_matches) {
            if (pair.second.length >= min_length) {
                // Double check min_length
                result.push_back(pair.second);
            }
        }

        // Sort by length descending, then by doc IDs
        std::sort(result.begin(), result.end());

        return result;
    }
} // namespace text_processing
