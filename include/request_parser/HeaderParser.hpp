#pragma once

#include <map>
#include <string>
#include <vector>

class HeaderParser
{
  public:
	std::multimap<std::string, std::string>
	static parseHeaders(std::istream &requestStream);

  private:
	HeaderParser(void);
	HeaderParser(const HeaderParser &src);
	~HeaderParser(void);
	HeaderParser &operator=(const HeaderParser &rhs);

	// Headers checks
	static void
	checkRawHeaders(const std::multimap<std::string, std::string> &headers);
	static void checkSingleHeader(std::string &headerLine);
	static bool isValidHeaderName(std::string headerName);
	static bool isValidHeaderValue(std::string headerValue);
	// clang-format off
	static void checkTokenSyntax(std::map<std::string, std::vector<std::string> >
							  headers);
	// clang-format on
};
