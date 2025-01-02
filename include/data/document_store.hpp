#ifndef TEXT_PROCESSING_DOCUMENT_STORE_HPP
#define TEXT_PROCESSING_DOCUMENT_STORE_HPP

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "text_processing/utf8_handler.hpp"

namespace text_processing {

/**
 * @brief Structure representing a document's position in concatenated text
 */
struct DocumentPosition {
    int64_t sql_id;         ///< SQL database ID of the document
    size_t start_pos;       ///< Start position in concatenated text
    size_t length;          ///< Length in UTF-8 characters
};

/**
 * @brief Class for managing documents and their positions
 */
class DocumentStore {
public:
    /**
     * @brief Constructor
     * @param separator UTF-8 character to use as document separator
     */
    explicit DocumentStore(const UTF8String& separator = UTF8String("|"));

    // Prevent copying, allow moving
    DocumentStore(const DocumentStore&) = delete;
    DocumentStore& operator=(const DocumentStore&) = delete;
    DocumentStore(DocumentStore&&) noexcept = default;
    DocumentStore& operator=(DocumentStore&&) noexcept = default;

    /**
     * @brief Add a new document to the store
     * @param content Document content
     * @param sql_id SQL database ID of the document
     * @return true if document was added successfully
     */
    bool add_document(const UTF8String& content, int64_t sql_id);

    /**
     * @brief Find which document contains a given position
     * @param pos Position in concatenated text
     * @return DocumentPosition structure with position information
     * @throw std::out_of_range if position is not found in any document
     */
    [[nodiscard]] DocumentPosition find_document_id(size_t pos) const;

    /**
     * @brief Get concatenated text of all documents
     */
    [[nodiscard]] const UTF8String& get_concatenated_text() const { return concatenated_text_; }

private:
    UTF8String separator_;                      ///< Document separator character
    UTF8String concatenated_text_;              ///< All documents concatenated
    std::vector<DocumentPosition> documents_;    ///< Document positions

    /**
     * @brief Rebuild concatenated text after adding documents
     */
    void rebuild_concatenated_text();
};

} // namespace text_processing

#endif // TEXT_PROCESSING_DOCUMENT_STORE_HPP