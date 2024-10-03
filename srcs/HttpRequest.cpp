#include "../include/HttpRequest.hpp"
#include "macros.hpp"
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <map>
#include <ostream>
#include <string>

HttpRequest::HttpRequest(
	std::string &method,
	std::string &httpVersion,
	std::string &uri,
	// clang-format off
	std::map<std::string, std::vector<std::string> > &headers,
	// clang-format on
	std::vector<char> &body
)
{
	normalizeRequest_(method, httpVersion, uri, headers, body);

	return;
}

HttpRequest::HttpRequest(const HttpRequest &src)
{
	*this = src;

	return;
}

HttpRequest &HttpRequest::operator=(const HttpRequest &rhs)
{
	method_ = rhs.getMethod();
	httpVersion_ = rhs.getHttpVersion();
	target_ = rhs.getTarget();
	uri_ = rhs.getUri();
	host_ = rhs.getHost();
	port_ = rhs.getPort();
	headers_ = rhs.getHeaders();
	body_ = rhs.getBody();
	keepAlive_ = rhs.getKeepAlive();
	hasCookie_ = rhs.hasCookie();
	cookie_ = rhs.getCookie();
	hasFileName_ = rhs.hasFileName();
	fileName_ = rhs.getFileName();

	return *this;
}

HttpRequest::~HttpRequest(void)
{
	return;
}

void HttpRequest::setMethod(std::string &newMethod)
{
	method_ = newMethod;
}

void HttpRequest::setHttpVersion(std::string &newHttpVersion)
{
	httpVersion_ = newHttpVersion;
}

void HttpRequest::setTarget(std::string &newTarget)
{
	target_ = newTarget;
}

void HttpRequest::setUri(std::string &newUri)
{
	uri_ = newUri;
}

void HttpRequest::setHeaders(
	// clang-format off
	std::map<std::string, std::vector<std::string> > &newHeaders
	// clang-format on
)
{
	headers_ = newHeaders;
}

void HttpRequest::setBody(std::vector<char> &newBody)
{
	body_ = newBody;
}

const std::string &HttpRequest::getMethod(void) const
{
	return method_;
}

const std::string &HttpRequest::getHttpVersion(void) const
{
	return httpVersion_;
}

const std::string &HttpRequest::getTarget(void) const
{
	return target_;
}

const std::string &HttpRequest::getUri(void) const
{
	return uri_;
}

const std::string &HttpRequest::getHost(void) const
{
	return host_;
}

const unsigned long &HttpRequest::getPort(void) const
{
	return port_;
}

// clang-format off
const std::map<std::string, std::vector<std::string> > &
// clang-format on
HttpRequest::getHeaders(void) const
{
	return headers_;
}

const std::vector<char> &HttpRequest::getBody(void) const
{
	return body_;
}

const bool &HttpRequest::getKeepAlive(void) const
{
	return keepAlive_;
}

const bool &HttpRequest::hasCookie(void) const
{
	return hasCookie_;
}

const std::string &HttpRequest::getCookie(void) const
{
	return cookie_;
}

const bool &HttpRequest::hasFileName(void) const
{
	return hasFileName_;
}

const std::string &HttpRequest::getFileName(void) const
{
	return fileName_;
}

std::ostream &operator<<(std::ostream &os, const HttpRequest &rhs)
{
	os << "Method: " << rhs.getMethod() << std::endl;
	os << "HTTP version: " << rhs.getHttpVersion() << std::endl;
	os << "Target: " << rhs.getTarget() << std::endl;
	os << "URI: " << rhs.getUri() << std::endl;
	os << "HEADERS:" << std::endl;
	// clang-format off
	const std::map<std::string, std::vector<std::string> > &headersCpy
		= rhs.getHeaders();
	for (std::map<std::string, std::vector<std::string> >::const_iterator it
		 = headersCpy.begin();
		// clang-format on
		it != headersCpy.end();
		it++)
	{
		if (it->second.size() == 1)
		{
			os << it->first << " : " << it->second[0] << std::endl;
		}
		else
		{
		}
	}
	os << "BODY:" << std::endl;
	const std::vector<char> &bodyCpy = rhs.getBody();
	for (std::vector<char>::const_iterator it = bodyCpy.begin();
		 it != bodyCpy.end();
		 it++)
	{
		os << *it;
	}
	os << std::endl;

	return os;
}

/**
 * @brief Extracts the port number from a given string.
 *
 * @param str Pointer to the string from which to extract the port number.
 * @return The extracted port number, or the default port if no valid
 *         port number is found.
 *
 * @note This function currently extracts the port from the Host header.
 *       In the future, it can be extended to also extract the port from
 *       the URI if needed.
 */
unsigned long HttpRequest::extractPort_(std::string *str)
{
	unsigned long port = DEFAULT_PORT; // default port
	size_t		  colonPos = str->find_last_of(':');
	if (colonPos != std::string::npos)
	{
		std::string tmpPort = host_.substr(colonPos + 1);
		bool		isDigit = true;
		for (size_t i = 0; i < tmpPort.length(); ++i)
		{
			if (!isdigit(tmpPort[i]))
			{
				isDigit = false;
				break;
			}
		}
		if (isDigit && !tmpPort.empty())
		{
			port = atol(tmpPort.c_str());
			*str = str->substr(0, colonPos);
		}
	}
	return port;
}

/**
 * @brief Determines if the connection should be kept alive.
 *
 * @return True if the "Connection" header is set to "keep-alive",
 *         otherwise false.
 *
 * @note This function checks the "Connection" header to determine if
 *       the client has requested to keep the connection alive.
 */
bool HttpRequest::extractKeepAlive_(void)
{
	if (headers_.count("Connection") > 0
		&& headers_.at("Connection")[0].compare("keep-alive") == 0)
	{
		return true;
	}

	return false;
}

/**
 * @brief Extracts the cookie from the request headers.
 *
 * @return The extracted cookie as a string. If no cookie is found,
 *         an empty string is returned.
 *
 * @note This function checks both "Cookie" and "cookie" headers to
 *       account for case variations. It also sets the hasCookie_
 *       member variable to indicate whether a cookie was found.
 */
std::string HttpRequest::extractCookie_(void)
{
	std::string cookie;

	if (headers_.count("Cookie") > 0)
	{
		hasCookie_ = true;
		return headers_.at("Cookie")[0];
	}
	if (headers_.count("cookie") > 0)
	{
		hasCookie_ = true;
		return headers_.at("cookie")[0];
	}
	hasCookie_ = false;

	return cookie;
}

std::string HttpRequest::extractFileName_(void)
{
	std::string fileName;

	size_t delimeter = uri_.find('?');
	if (delimeter != std::string::npos)
	{
		fileName = uri_.substr(delimeter, uri_.length() - delimeter);
		uri_ = uri_.substr(delimeter);
	}

	hasFileName_ = false;
	return fileName;
}


/**
 * @brief Normalizes the HTTP request by extracting and storing relevant
 * components.
 *
 * This function takes the method, HTTP version, URI, headers, and body of an
 * HTTP request and normalizes them by extracting and storing the relevant
 * components into the HttpRequest object. It also extracts the port number from
 * the Host header using the `extractPort` function and extracts cookies from
 * the request headers using the `extractCookie` function.
 *
 * @param method The HTTP method (e.g., GET, POST).
 * @param httpVersion The HTTP version (e.g., HTTP/1.1).
 * @param uri The URI of the request.
 * @param inputHeaders A map of the request headers.
 * @param body The body of the request.
 *
 * @note The port extraction is currently done from the Host header. In the
 * future, this can be extended to also extract the port from the URI if needed.
 *
 * @note The cookie extraction checks both "Cookie" and "cookie" headers to
 * account for case variations.
 */
void HttpRequest::normalizeRequest_(
	std::string &method,
	std::string &httpVersion,
	std::string &uri,
	// clang-format off
	std::map<std::string, std::vector<std::string> > &inputHeaders,
	// clang-format on
	std::vector<char> &body
)
{
	method_ = method;
	httpVersion_ = httpVersion;
	uri_ = uri;
	fileName_ = extractFileName_();
	host_ = inputHeaders.at("Host")[0];
	port_ = extractPort_(&host_);
	target_ = host_ + uri_;
	headers_ = inputHeaders;
	body_ = body;
	keepAlive_ = extractKeepAlive_();
	cookie_ = extractCookie_();
}
