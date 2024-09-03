#pragma once

#include <map>
#include <string>
#include <vector>

// TODO: implement normalization on initialization
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
	HttpRequest(const HttpRequest &src);
	~HttpRequest(void);

	// Setters
	void setMethod(std::string &newMethod);
	void setHttpVersion(std::string &newHttpVersion);
	void setTarget(std::string &newTarget);
	void setHeaders(std::map<std::string, std::string> &newHeaders);
	void setBody(std::vector<char> &newBody);

	// Getters
	const std::string						 &getMethod(void) const;
	const std::string						 &getHttpVersion(void) const;
	const std::string						 &getTarget(void) const;
	const std::map<std::string, std::string> &getHeaders(void) const;
	const std::vector<char>					 &getBody(void) const;

	// Overloaded Operators
	HttpRequest &operator=(const HttpRequest &rhs);

  private:
	std::string						   method_;
	std::string						   httpVersion_;
	std::string						   target_;
	std::map<std::string, std::string> headers_;
	std::vector<char>				   body_;
};

std::ostream &operator<<(std::ostream &os, const HttpRequest &rhs);
