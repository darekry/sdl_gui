#pragma once

namespace utf8 {

inline size_t charToByteIndex(std::string_view text, size_t charIndex) {
    size_t byteIdx = 0;
    size_t charCount = 0;
    
    while (byteIdx < text.size() && charCount < charIndex) {
        unsigned char c = static_cast<unsigned char>(text[byteIdx]);
        if ((c & 0x80) == 0) byteIdx += 1;
        else if ((c & 0xE0) == 0xC0) byteIdx += 2;
        else if ((c & 0xF0) == 0xE0) byteIdx += 3;
        else if ((c & 0xF8) == 0xF0) byteIdx += 4;
        else byteIdx += 1;
        charCount++;
    }
    return byteIdx;
}

inline size_t charCount(std::string_view text) {
    size_t count = 0;
    size_t byteIdx = 0;
    
    while (byteIdx < text.size()) {
        unsigned char c = static_cast<unsigned char>(text[byteIdx]);
        if ((c & 0x80) == 0) byteIdx += 1;
        else if ((c & 0xE0) == 0xC0) byteIdx += 2;
        else if ((c & 0xF0) == 0xE0) byteIdx += 3;
        else if ((c & 0xF8) == 0xF0) byteIdx += 4;
        else byteIdx += 1;
        count++;
    }
    return count;
}

inline size_t charByteLength(unsigned char firstByte) {
    if ((firstByte & 0x80) == 0) return 1;
    if ((firstByte & 0xE0) == 0xC0) return 2;
    if ((firstByte & 0xF0) == 0xE0) return 3;
    if ((firstByte & 0xF8) == 0xF0) return 4;
    return 1;
}

inline std::string substrChars(std::string_view text, size_t charStart, size_t charLen) {
    size_t byteStart = charToByteIndex(text, charStart);
    size_t byteEnd = charToByteIndex(text, charStart + charLen);
    return std::string(text.substr(byteStart, byteEnd - byteStart));
}

inline size_t prevCharBytePos(std::string_view text, size_t currentBytePos) {
    if (currentBytePos == 0) return 0;
    
    size_t pos = currentBytePos - 1;
    while (pos > 0 && (text[pos] & 0xC0) == 0x80) {
        pos--;
    }
    return pos;
}

inline size_t nextCharBytePos(std::string_view text, size_t currentBytePos) {
    if (currentBytePos >= text.size()) return text.size();
    return currentBytePos + charByteLength(static_cast<unsigned char>(text[currentBytePos]));
}

inline size_t byteIndexToCharIndex(std::string_view text, size_t byteIndex) {
    size_t charIdx = 0;
    size_t bytePos = 0;
    
    while (bytePos < byteIndex && bytePos < text.size()) {
        unsigned char c = static_cast<unsigned char>(text[bytePos]);
        if ((c & 0x80) == 0) bytePos += 1;
        else if ((c & 0xE0) == 0xC0) bytePos += 2;
        else if ((c & 0xF0) == 0xE0) bytePos += 3;
        else if ((c & 0xF8) == 0xF0) bytePos += 4;
        else bytePos += 1;
        charIdx++;
    }
    return charIdx;
}

} // namespace utf8