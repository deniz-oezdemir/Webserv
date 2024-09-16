#pragma once

#include <map>
#include <string>
#include <vector>

class HeaderParser
{
  public:
	void static parseHeaders(
		std::istream &requestStream,
		// clang-format off
	std::map<std::string, std::vector<std::string> > *headers
		// clang-format on
	);

  private:
	HeaderParser(void);
	HeaderParser(const HeaderParser &src);
	~HeaderParser(void);
	HeaderParser &operator=(const HeaderParser &rhs);

	// Headers checks
	static void
	checkRawHeaders_(const std::multimap<std::string, std::string> &headers);
	static void checkSingleHeader_(std::string &headerLine);
	static bool isValidHeaderName_(std::string headerName);
	static bool isValidHeaderValue_(std::string headerValue);
	// clang-format off
	static void checkTokenSyntax_(std::map<std::string, std::vector<std::string> >
							  headers);
	// clang-format on

	// HttpRequest creation
	// clang-format off
	static std::map<std::string, std::vector<std::string> >
	unifyHeaders_(std::multimap<std::string, std::string> multimap);
	// clang-format on
	static std::vector<std::string> splitHeaderValue_(
		const std::string &headerName,
		const std::string &headerValue
	);
};
