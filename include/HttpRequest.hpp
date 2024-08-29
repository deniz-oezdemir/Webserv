#pragma once

#include <map>
#include <string>
#include <sstream>
#include <vector>

class HttpRequest
{
  public:
	HttpRequest(
		std::string						   &method,
		std::string						   &httpVersion,
		std::string						   &target,
		std::map<std::string, std::string> &headers,
		std::vector<char>				   &body
	);
	HttpRequest(const HttpRequest& src);
	~HttpRequest(void);

	// Setters
	void setMethod(std::string &newMethod);
	void setHttpVersion(std::string &newHttpVersion);
	void setTarget(std::string &newTarget);
	void setHeaders(std::map<std::string, std::string> &newHeaders);
	void setBody(std::vector<char> &newBody);

	// Getters
	const std::string&						   getMethod(void) const;
	const std::string&						   getHttpVersion(void) const;
	const std::string&						   getTarget(void) const;
	const std::map<std::string, std::string>& getHeaders(void) const;
	const std::vector<char>&				   getBody(void) const;

	// Overloaded Operators
	HttpRequest&	 operator=(const HttpRequest& rhs);

  private:
	std::string						   _method;
	std::string						   _httpVersion;
	std::string						   _target;
	std::map<std::string, std::string> _headers;
	std::vector<char>				   _body;
};

std::ostringstream& operator<<(std::ostringstream& os, const HttpRequest& rhs);
