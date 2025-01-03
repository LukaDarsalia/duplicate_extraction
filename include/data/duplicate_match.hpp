#ifndef DUPLICATE_MATCH_HPP
#define DUPLICATE_MATCH_HPP
#include <cstdint>
#include <sstream>


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

        [[nodiscard]] std::string to_json() const {
            std::ostringstream json;
            json << "{"
                    << "\"doc1_id\": " << doc1_id << ", "
                    << "\"doc2_id\": " << doc2_id << ", "
                    << "\"start_pos1\": " << start_pos1 << ", "
                    << "\"start_pos2\": " << start_pos2 << ", "
                    << "\"length\": " << length
                    << "}";
            return json.str();
        }

        /**
         * @brief Convert a vector of Matches to JSON array string
         * @param matches Vector of matches to convert
         * @return JSON array string of matches
         */
        static std::string to_json_array(const std::vector<Match> &matches) {
            std::ostringstream json;
            json << "[";
            for (size_t i = 0; i < matches.size(); ++i) {
                if (i > 0) json << ", ";
                json << matches[i].to_json();
            }
            json << "]";
            return json.str();
        }
    };

#endif //DUPLICATE_MATCH_HPP
}
