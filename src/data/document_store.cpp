#include "data/document_store.hpp"
#include <stdexcept>
#include <algorithm>

namespace text_processing {
    DocumentStore::DocumentStore(UTF8String separator)
        : separator_(std::move(separator))
          , concatenated_text_(UTF8String("")) {
    }

    std::set<DocumentPosition>::const_iterator DocumentStore::find_by_sql_id(int64_t sql_id) const {
        // Binary search in documents_ (sorted by sql_id)
        auto it = std::lower_bound(documents_.begin(), documents_.end(),
                                   DocumentPosition{sql_id, 0, 0},
                                   [](const DocumentPosition &a, const DocumentPosition &b) {
                                       return a.sql_id < b.sql_id;
                                   });

        if (it != documents_.end() && it->sql_id == sql_id) {
            return it;
        }
        return documents_.end();
    }

    bool DocumentStore::add_document(const UTF8String &content, int64_t sql_id) {
        // Check if document exists using binary search - O(log n)
        if (find_by_sql_id(sql_id) != documents_.end()) {
            return false;
        }

        // Calculate start position for new document
        size_t start_pos = concatenated_text_.length();

        // Create new document position
        DocumentPosition doc_pos{
            sql_id,
            start_pos,
            content.length()
        };

        // Insert into documents_ maintaining sorted order by sql_id - O(log(n))
        auto insert_pos = std::lower_bound(documents_.begin(), documents_.end(), doc_pos);
        documents_.insert(insert_pos, doc_pos);

        // Insert into pos_index_ maintaining sorted order by start_pos - O(1)
        pos_index_.push_back(doc_pos); // For positions, we can just append as it's always at the end

        // Update concatenated text
        concatenated_text_ = concatenated_text_ + content + separator_;

        return true;
    }

    DocumentPosition DocumentStore::find_document_id(size_t pos) const {
        if (pos_index_.empty()) {
            throw std::out_of_range("No documents in store");
        }

        // Binary search based on document start positions in pos_index_
        auto it = std::upper_bound(pos_index_.begin(), pos_index_.end(), pos,
                                   [](size_t pos, const DocumentPosition &doc) {
                                       return pos < doc.start_pos;
                                   });

        if (it != pos_index_.begin()) {
            --it;
        } else {
            throw std::out_of_range("Position before first document");
        }

        // Check if position is within document bounds (including separator)
        size_t doc_end = it->start_pos + it->length;
        if (it != --pos_index_.end()) {
            // If not last document, include separator
            doc_end += separator_.length();
        }

        if (pos >= it->start_pos && pos < doc_end) {
            return *it;
        }

        throw std::out_of_range("Position not found in any document");
    }
} // namespace text_processing
