#ifndef TEXT_PROCESSING_UTF8_HANDLER_HPP
#define TEXT_PROCESSING_UTF8_HANDLER_HPP

#include <string>
#include <vector>
#include <stdexcept>

namespace text_processing {
    /**
     * @brief Exception class for UTF-8 related errors
     */
    class UTF8Error : public std::runtime_error {
    public:
        explicit UTF8Error(const std::string &message) : std::runtime_error(message) {
        }
    };

    /**
     * @brief Class representing a UTF-8 string with Python-like character access
     *
     * This class wraps a UTF-8 encoded string and provides an interface to access
     * individual Unicode characters similar to how Python handles strings.
     *
     * Example:
     * @code
     *     UTF8String str("გამარჯობა");
     *     assert(str[0] == "გ");
     *     assert(str[1] == "ა");
     *
     *     for(const auto& ch : str) {
     *         std::cout << ch << ' ';  // Prints each character separated by space
     *     }
     * @endcode
     */
    class UTF8String {
    public:
        class Character; // Forward declaration
        class Iterator; // Forward declaration

        /**
        * @brief Default constructor creates an empty string
        */
        UTF8String() : data_(), char_count_(0) {
        }

        /**
         * @brief Construct UTF8String from std::string
         * @throws UTF8Error if input is not valid UTF-8
         */
        explicit UTF8String(const std::string &str);

        /**
         * @brief Get character at specified index
         * @throws std::out_of_range if index is invalid
         */
        Character operator[](size_t index) const;

        /**
         * @brief Get number of characters in string
         */
        [[nodiscard]] size_t length() const { return char_count_; }

        /**
         * @brief Get underlying UTF-8 encoded string
         */
        [[nodiscard]] const std::string &str() const { return data_; }

        /**
         * @brief Iterator support
         */
        [[nodiscard]] Iterator begin() const;

        [[nodiscard]] Iterator end() const;

        /**
         * @brief Substring operation
         * @param start Start character index
         * @param length Number of characters (not bytes)
         */
        [[nodiscard]] UTF8String substr(size_t start, size_t length) const;

        /**
        * @brief String concatenation
        */
        UTF8String operator+(const UTF8String &other) const;

        /**
         * @brief Equality comparison
         */
        bool operator==(const UTF8String &other) const;

        bool operator!=(const UTF8String &other) const;

        /**
         * @brief Less than comparison (lexicographical)
         */
        bool operator<(const UTF8String &other) const;

    private:
        std::string data_; // Raw UTF-8 encoded string
        std::vector<size_t> char_pos_; // Start positions of each character
        size_t char_count_; // Number of characters

        void indexString(); // Build character position index
    };

    /**
     * @brief Represents a single Unicode character in UTF-8 encoding
     */
    class UTF8String::Character {
    public:
        /**
        * @brief Default constructor creates an empty character
        */
        Character() : data_() {
        }

        /**
         * @brief Get string representation of the character
         */
        [[nodiscard]] std::string str() const { return data_; }

        /**
         * @brief Comparison operators
         */
        bool operator==(const Character &other) const;

        bool operator!=(const Character &other) const;

        bool operator<(const Character &other) const;

    private:
        friend class UTF8String;
        std::string data_; // The UTF-8 encoded character
        explicit Character(std::string ch) : data_(std::move(ch)) {
        }
    };

    /**
     * @brief Iterator for UTF8String
     */
    class UTF8String::Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = Character;
        using difference_type = std::ptrdiff_t;
        using pointer = const Character *;
        using reference = const Character &;

        /**
         * @brief Default constructor
         */
        Iterator() : str_(nullptr), pos_(0), current_("") {
        }

        Iterator &operator++();

        Iterator operator++(int);

        bool operator==(const Iterator &other) const;

        bool operator!=(const Iterator &other) const;

        reference operator*() const;

    private:
        friend class UTF8String;
        const UTF8String *str_;
        size_t pos_;
        Character current_;

        Iterator(const UTF8String *str, size_t pos);
    };
} // namespace text_processing

#endif // TEXT_PROCESSING_UTF8_HANDLER_HPP
