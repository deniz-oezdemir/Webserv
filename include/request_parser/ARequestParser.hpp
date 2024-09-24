#pragma once

#include "HttpRequest.hpp"

/* @class ARequestParser
 * @brief Parses and validates the syntax of HTTP requests.
 *
 * The ARequestParser class is responsible for checking the syntax of the
 * received HTTP request. It ensures that the request adheres to the HTTP/1.1
 * specification in terms of structure and format. However, it does not validate
 * the actual content of the fields. For example, a URI might pass all syntax
 * checks but still not correspond to any valid target on the server. On
 * successful parsing, ARequestParser returns an HttpRequest object.
 */
class ARequestParser
{
  public:
	static HttpRequest parseRequest(std::string str);

  private:
	ARequestParser(void);
	~ARequestParser(void);
	ARequestParser(const ARequestParser &src);
	ARequestParser &operator=(const ARequestParser &rhs);
};
