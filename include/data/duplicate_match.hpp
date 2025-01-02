#ifndef DUPLICATE_MATCH_HPP
#define DUPLICATE_MATCH_HPP
#include <cstdint>


namespace text_processing {
    /**
     * @brief Structure representing a duplicate text match between two documents
     */
    struct Match {
        int64_t doc1_id; ///< SQL ID of first document
        int64_t doc2_id; ///< SQL ID of second document
        size_t start_pos1; ///< Start position in first document
        size_t start_pos2; ///< Start position in second document
        size_t length; ///< Length of the common substring

        // Comparison operators for sorting and containers
        bool operator<(const Match &other) const {
            if (length != other.length) return length > other.length; // Sort by length descending
            if (doc1_id != other.doc1_id) return doc1_id < other.doc1_id;
            return doc2_id < other.doc2_id;
        }

        bool operator==(const Match &other) const {
            return doc1_id == other.doc1_id &&
                   doc2_id == other.doc2_id &&
                   start_pos1 == other.start_pos1 &&
                   start_pos2 == other.start_pos2 &&
                   length == other.length;
        }
    };

#endif //DUPLICATE_MATCH_HPP
}
