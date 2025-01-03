#include "text_processing/utf8_handler.hpp"

namespace text_processing {
    // Character class implementations
    bool UTF8String::Character::operator==(const Character &other) const {
        return data_ == other.data_;
    }

    bool UTF8String::Character::operator!=(const Character &other) const {
        return !(*this == other);
    }

    bool UTF8String::Character::operator<(const Character &other) const {
        return data_ < other.data_;
    }

    // Iterator class implementations
    UTF8String::Iterator::Iterator(const UTF8String *str, size_t pos)
        : str_(str), pos_(pos), current_("") {
        if (str_ && pos_ < str_->length()) {
            current_ = (*str_)[pos_];
        }
    }

    UTF8String::Iterator &UTF8String::Iterator::operator++() {
        if (str_ && pos_ < str_->length()) {
            ++pos_;
            if (pos_ < str_->length()) {
                current_ = (*str_)[pos_];
            }
        }
        return *this;
    }

    UTF8String::Iterator UTF8String::Iterator::operator++(int) {
        Iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    bool UTF8String::Iterator::operator==(const Iterator &other) const {
        return str_ == other.str_ && pos_ == other.pos_;
    }

    bool UTF8String::Iterator::operator!=(const Iterator &other) const {
        return !(*this == other);
    }

    UTF8String::Iterator::reference UTF8String::Iterator::operator*() const {
        return current_;
    }

    // UTF8String class implementations
    UTF8String::UTF8String(const std::string &str) : data_(str), char_count_(0) {
        indexString();
    }

    void UTF8String::indexString() {
        char_pos_.clear();
        char_count_ = 0;

        if (data_.empty()) {
            return;
        }

        const auto *bytes = reinterpret_cast<const unsigned char *>(data_.c_str());
        size_t len = data_.length();
        size_t pos = 0;

        while (pos < len) {
            char_pos_.push_back(pos);

            // Get number of bytes in current character
            unsigned char first_byte = bytes[pos];
            int char_bytes;

            if ((first_byte & 0x80) == 0) {
                // Single byte character (0xxxxxxx)
                char_bytes = 1;
            } else if ((first_byte & 0xE0) == 0xC0) {
                // Two byte character (110xxxxx)
                char_bytes = 2;
            } else if ((first_byte & 0xF0) == 0xE0) {
                // Three byte character (1110xxxx)
                char_bytes = 3;
            } else if ((first_byte & 0xF8) == 0xF0) {
                // Four byte character (11110xxx)
                char_bytes = 4;
            } else {
                // Invalid UTF-8 sequence
                throw UTF8Error("Invalid UTF-8 sequence at position " + std::to_string(pos));
            }

            // Check if we have enough bytes
            if (pos + char_bytes > len) {
                throw UTF8Error("Truncated UTF-8 sequence at position " + std::to_string(pos));
            }

            // Validate continuation bytes
            for (int i = 1; i < char_bytes; i++) {
                if ((bytes[pos + i] & 0xC0) != 0x80) {
                    throw UTF8Error("Invalid UTF-8 continuation byte at position " +
                                    std::to_string(pos + i));
                }
            }

            // Validate against overlong encodings
            switch (char_bytes) {
                case 2:
                    if ((first_byte & 0x1E) == 0) {
                        throw UTF8Error("Overlong UTF-8 encoding at position " +
                                        std::to_string(pos));
                    }
                    break;
                case 3:
                    if (first_byte == 0xE0 && (bytes[pos + 1] & 0x20) == 0) {
                        throw UTF8Error("Overlong UTF-8 encoding at position " +
                                        std::to_string(pos));
                    }
                    break;
                case 4:
                    if (first_byte == 0xF0 && (bytes[pos + 1] & 0x30) == 0) {
                        throw UTF8Error("Overlong UTF-8 encoding at position " +
                                        std::to_string(pos));
                    }
                    break;
                default: break;
            }

            pos += char_bytes;
            char_count_++;
        }
    }

    UTF8String::Character UTF8String::operator[](size_t index) const {
        if (index >= char_count_) {
            throw std::out_of_range("Character index out of range");
        }

        size_t start = char_pos_[index];
        size_t length;

        if (index + 1 < char_count_) {
            length = char_pos_[index + 1] - start;
        } else {
            length = data_.length() - start;
        }

        return Character(data_.substr(start, length));
    }

    UTF8String::Iterator UTF8String::begin() const {
        return {this, 0};
    }

    UTF8String::Iterator UTF8String::end() const {
        return {this, char_count_};
    }

    UTF8String UTF8String::substr(size_t start, size_t length) const {
        if (start > char_count_) {
            throw std::out_of_range("Start index out of range");
        }
        if (start + length > char_count_) {
            throw std::out_of_range("Substring length too large");
        }
        if (length == 0) {
            return UTF8String("");
        }

        size_t begin_pos = char_pos_[start];
        size_t end_pos;

        if (start + length < char_count_) {
            end_pos = char_pos_[start + length];
        } else {
            end_pos = data_.length();
        }

        return UTF8String(data_.substr(begin_pos, end_pos - begin_pos));
    }

    bool UTF8String::operator==(const UTF8String &other) const {
        return data_ == other.data_;
    }

    bool UTF8String::operator!=(const UTF8String &other) const {
        return !(*this == other);
    }

    bool UTF8String::operator<(const UTF8String &other) const {
        return data_ < other.data_;
    }

    UTF8String UTF8String::operator+(const UTF8String &other) const {
        return UTF8String(data_ + other.data_);
    }

    UTF8String &UTF8String::operator+=(const UTF8String &other) {
        if (other.data_.empty()) {
            return *this;
        }

        size_t original_size = data_.length();
        size_t original_char_count = char_count_;

        // Add the new data
        data_ += other.data_;

        // Add positions, offsetting by original size
        char_pos_.reserve(char_pos_.size() + other.char_pos_.size());
        for (size_t pos: other.char_pos_) {
            char_pos_.push_back(pos + original_size);
        }

        char_count_ = original_char_count + other.char_count_;
        return *this;
    }

    UTF8String &UTF8String::operator+=(const std::string &other) {
        if (other.empty()) {
            return *this;
        }

        // Create temporary UTF8String to validate and get positions
        UTF8String temp(other);
        return operator+=(temp);
    }
} // namespace text_processing
