#pragma once
#include "HttpException.hpp"
#include "HttpRequest.hpp"
#include "Logger.hpp"
#include "macros.hpp"
#include <algorithm>
#include <cstddef>
#include <iostream>

// RequestParser checks the syntax of the received request. It does not check
// the validity of the fields. For example, a target's URI might pass all syntax
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
		std::string *target,
		std::string *httpVersion
	);
	static void checkMethod(std::string &method);
	static void checkTarget(std::string &target);
	static void checkHttpVersion(std::string &httpVersion);

	// Headers checks
	void checkHeaders(const std::map<std::string, std::string> &headers);
};
