#pragma once

#include <map>
#include <string>

#define REPEATABLEHEADERS_N 20
#define SEMICOLONSEPARATE_N 5

extern const std::string repeatableHeaders[REPEATABLEHEADERS_N];
extern const std::string semicolonSeparated[SEMICOLONSEPARATE_N];
extern const std::map<std::string, std::string> headerAcceptedChars;

std::map<std::string, std::string> createHeaderAcceptedChars();
