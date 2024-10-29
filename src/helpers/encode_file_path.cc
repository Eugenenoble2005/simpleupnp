#include <sstream>
#include <iomanip>
#include <string>
#include "encode_file_path.h"
#include <iostream>
std::string EncodeFilePath(std::string& file_path) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (char c : file_path) {
        // Encode characters that are not alphanumeric or safe characters.
        if (isalnum(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << c;
        } else {
            // Convert each byte to its percent-encoded form.
            escaped << '%' << std::setw(2) << static_cast<int>(static_cast<unsigned char>(c));
        }
    }
    return escaped.str();
}

std::string DecodeFilePath(std::string& encoded_file_path) {
    std::ostringstream unescaped;
    size_t             i = 0;
    while (i < encoded_file_path.length()) {
        if (encoded_file_path[i] == '%' && i + 2 < encoded_file_path.length()) {
            std::string hex         = encoded_file_path.substr(i + 1, 2);
            char        decodedChar = static_cast<char>(std::stoi(hex, nullptr, 16));
            unescaped << decodedChar;
            i += 3;
        } else if (encoded_file_path[i] == '+') {
            unescaped << ' ';
            i++;
        } else {
            unescaped << encoded_file_path[i];
            i++;
        }
    }
    return unescaped.str();
}
