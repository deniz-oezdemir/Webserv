#pragma once

#include "HttpRequest.hpp"
#include <map>
#include <vector>

/* @class RequestParser
 * @brief Parses and validates the syntax of HTTP requests.
 *
 * The RequestParser class is responsible for checking the syntax of the
 * received HTTP request. It ensures that the request adheres to the HTTP/1.1
 * specification in terms of structure and format. However, it does not validate
 * the actual content of the fields. For example, a URI might pass all syntax
 * checks but still not correspond to any valid target on the server. On
 * successful parsing, RequestParser returns an HttpRequest object.
 */
class RequestParser
{
  public:
	static HttpRequest parseRequest(std::string str);

  private:
	RequestParser(void);
	~RequestParser(void);
	RequestParser(const RequestParser &src);
	RequestParser &operator=(const RequestParser &rhs);

	// Start line checks
	static void checkStartLine_(
		std::string &startLine,
		std::string *method,
		std::string *uri,
		std::string *httpVersion
	);
	static void checkMethod_(std::string &method);
	static void checkUri(std::string &uri);
	static void checkHttpVersion(std::string &httpVersion);

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

	// Body checks
	static void checkBody(
		const std::string &method,
		// clang-format off
		const std::map<std::string, std::vector<std::string> > &headers,
		// clang-format on
		const std::vector<char> &body
	);

	// HttpRequestcreation
	// clang-format off
	static std::map<std::string, std::vector<std::string> >
	unifyHeaders_(std::multimap<std::string, std::string> multimap);
	// clang-format on
	static std::vector<std::string> splitHeaderValue_(
		const std::string &headerName,
		const std::string &headerValue
	);

	static const std::string repeatableHeaders[20];
	static const std::string semicolonSeparated[5];
	static const std::map<std::string, std::string> headerAcceptedChars_;
};
