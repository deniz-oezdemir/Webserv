#pragma once

#include <map>
#include <string>

/* WARNING: change length macros if headers are changed */

#define ACCEPTED_HEADERS_N 6
#define REPEATABLE_HEADERS_N 20
#define SEMICOLON_SEPARATED_N 5

extern const std::string acceptedHeaders[ACCEPTED_HEADERS_N];
extern const std::string repeatableHeaders[REPEATABLE_HEADERS_N];
extern const std::string semicolonSeparated[SEMICOLON_SEPARATED_N];
extern const std::string delimeterChars;
extern const std::map<std::string, std::string> headerAcceptedChars;

std::map<std::string, std::string> createHeaderAcceptedChars();
