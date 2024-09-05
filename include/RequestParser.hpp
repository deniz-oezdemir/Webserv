#pragma once
#include "HttpException.hpp"
#include "HttpRequest.hpp"

// RequestParser checks the syntax of the received request. It does not check
// the validity of the fields. For example, a URI might pass all syntax
// checks and still not correspond with any valid target in the server.
// On succes, RequestParser returns a HttpRequest object.
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
	static void checkStartLine(
		std::string &startLine,
		std::string *method,
		std::string *uri,
		std::string *httpVersion
	);
	static void checkMethod(std::string &method);
	static void checkUri(std::string &uri);
	static void checkHttpVersion(std::string &httpVersion);

	// Headers checks
	void checkHeaders(const std::multimap<std::string, std::string> &headers);

	// Body checks
	void checkBody(
		const std::string							  &method,
		const std::multimap<std::string, std::string> &headers,
		const std::string							  &body
	);
};
