#pragma once
#include "macros.hpp"
#include "HttpRequest.hpp"
#include "HttpException.hpp"
#include <cstddef>
#include <iostream>

class RequestParser
{
  public:
	static HttpRequest parseRequest(std::string str);

  private:
	RequestParser(void);
	~RequestParser(void);
	RequestParser(const RequestParser &src);
	RequestParser &operator=(const RequestParser &rhs);

	/*
	 * HTTP request syntax checks
	 */
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
};
