#include "text_processing/utf8_handler.hpp"

namespace text_processing {

// Character class implementations
bool UTF8String::Character::operator==(const Character& other) const {
    return data_ == other.data_;
}

bool UTF8String::Character::operator!=(const Character& other) const {
    return !(*this == other);
}

bool UTF8String::Character::operator<(const Character& other) const {
    return data_ < other.data_;
}

// Iterator class implementations
UTF8String::Iterator::Iterator(const UTF8String* str, size_t pos)
    : str_(str), pos_(pos), current_("") {
    if (str_ && pos_ < str_->length()) {
        current_ = (*str_)[pos_];
    }
}

UTF8String::Iterator& UTF8String::Iterator::operator++() {
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

bool UTF8String::Iterator::operator==(const Iterator& other) const {
    return str_ == other.str_ && pos_ == other.pos_;
}

bool UTF8String::Iterator::operator!=(const Iterator& other) const {
    return !(*this == other);
}

UTF8String::Iterator::reference UTF8String::Iterator::operator*() const {
    return current_;
}

// UTF8String class implementations
UTF8String::UTF8String(const std::string& str) : data_(str), char_count_(0) {
    indexString();
}

void UTF8String::indexString() {
    // This is a placeholder implementation
    // We'll implement proper UTF-8 handling here
    char_count_ = data_.length();  // Temporarily treat each byte as a character
    char_pos_.clear();
    for (size_t i = 0; i < data_.length(); ++i) {
        char_pos_.push_back(i);
    }
}

UTF8String::Character UTF8String::operator[](size_t index) const {
    if (index >= char_count_) {
        throw std::out_of_range("Character index out of range");
    }
    if (index + 1 < char_count_) {
        return Character(data_.substr(char_pos_[index],
                                    char_pos_[index + 1] - char_pos_[index]));
    }
    return Character(data_.substr(char_pos_[index]));
}

UTF8String::Iterator UTF8String::begin() const {
    return Iterator(this, 0);
}

UTF8String::Iterator UTF8String::end() const {
    return Iterator(this, char_count_);
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
    size_t end_pos = (start + length < char_count_)
                     ? char_pos_[start + length]
                     : data_.length();
    return UTF8String(data_.substr(begin_pos, end_pos - begin_pos));
}

bool UTF8String::operator==(const UTF8String& other) const {
    return data_ == other.data_;
}

bool UTF8String::operator!=(const UTF8String& other) const {
    return !(*this == other);
}

bool UTF8String::operator<(const UTF8String& other) const {
    return data_ < other.data_;
}

UTF8String UTF8String::operator+(const UTF8String& other) const {
    return UTF8String(data_ + other.data_);
}

} // namespace text_processing