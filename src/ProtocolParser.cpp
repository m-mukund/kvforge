#include "ProtocolParser.h"

std::pair<std::vector<std::string>, size_t> parse_resp(const std::string& buffer) {
    std::vector<std::string> tokens;
    size_t cursor = 0;

    // Sanity check
    if (buffer.empty() || buffer[cursor] != '*') return {{}, 0}; 

    // Find array length
    size_t first_crlf = buffer.find("\r\n", cursor);
    if (first_crlf == std::string::npos) return {{}, 0}; 

    int num_args = std::stoi(buffer.substr(1, first_crlf-1)); 
 
    cursor = first_crlf + 2;

    for (int i = 0; i < num_args; ++i) {
        if (cursor >= buffer.size() || buffer[cursor] != '$') break;

        size_t length_crlf = buffer.find("\r\n", cursor);
        if (length_crlf == std::string::npos) break;

        int str_len = std::stoi(buffer.substr(cursor+1, length_crlf-(cursor+1)));
        
        cursor = length_crlf + 2;

        // Safety Check before substr
        if (cursor + str_len > buffer.size())
            return {};

        tokens.push_back(buffer.substr(cursor, str_len)); 

        cursor = cursor + str_len + 2;
    }

    return {tokens, cursor};
}