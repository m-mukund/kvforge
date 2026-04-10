#ifndef PROTOCOL_PARSER_H
#define PROTOCOL_PARSER_H

#include <cstddef>
#include <utility>
#include <vector>
#include <string>

std::pair<std::vector<std::string>, size_t> parse_resp(const std::string& buffer);

#endif