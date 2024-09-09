#pragma once

#include <map>
#include <string>
#include <vector>

/**
 * @class HttpRequest
 * @brief Represents an HTTP request with all necessary information for
 * handling.
 *
 * The HttpRequest class encapsulates all the required information of an HTTP
 * request, including the method, HTTP version, URI, headers, and body. It is
 * designed to be constructed by the RequestParser after a request passes all
 * parsing tests. The class ensures consistency by normalizing itself upon
 * initialization, adhering to the HTTP/1.1 specification.
 */
class HttpRequest
{
  public:
	HttpRequest(
		std::string								&method,
		std::string								&httpVersion,
		std::string								&uri,
		std::multimap<std::string, std::string> &headers,
		std::vector<char>						&body
	);
	HttpRequest(const HttpRequest &src);
	~HttpRequest(void);

	// Setters
	void setMethod(std::string &newMethod);
	void setHttpVersion(std::string &newHttpVersion);
	void setTarget(std::string &newTarget);
	// clang-format off
	void setHeaders(std::map<std::string, std::vector<std::string> > &newHeaders
				 );
	// clang-format on
	void setBody(std::vector<char> &newBody);

	// Getters
	const std::string &getMethod(void) const;
	const std::string &getHttpVersion(void) const;
	const std::string &getTarget(void) const;
	// clang-format off
	const std::map<std::string, std::vector<std::string> > &getHeaders(void
	) const;
	// clang-format on
	const std::vector<char> &getBody(void) const;

	// Overloaded Operators
	HttpRequest &operator=(const HttpRequest &rhs);

  private:
	/**
	 * This method is used to normalize the HTTP request. It checks that all
	 * fields follow the correct syntax according to the HTTP/1.1 specification.
	 * It also sets the target of the request by combining the Host header and
	 * the URI.
	 */
	void normalizeRequest(
		std::string								&method,
		std::string								&httpVersion,
		std::string								&uri,
		std::multimap<std::string, std::string> &headers,
		std::vector<char>						&body
	);

	std::string method_;
	std::string httpVersion_;
	std::string target_;
	// clang-format off
	std::map<std::string, std::vector<std::string> > headers_;
	// clang-format on
	std::vector<char> body_;
};

std::ostream &operator<<(std::ostream &os, const HttpRequest &rhs);
