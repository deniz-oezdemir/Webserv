#include "../include/HttpRequest.hpp"
#include <cstdlib>
#include <ostream>

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
	method_ = src.method_;
	httpVersion_ = src.httpVersion_;
	target_ = src.target_;
	headers_ = src.headers_;
	body_ = src.body_;

	return;
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

	// Remove all precedent chars to the URI target, in order to be latter
	// appended to the Host. For ex: remoce the 'http://' part if any
	// and store the port if any.
	if (uri[0] != '*'
		&& (uri.find("https://") != std::string::npos
			|| uri.find("http://") != std::string::npos))
	{
		size_t schemeEnd = uri.find("://") + 3;		 // Find end of scheme
		size_t pathStart = uri.find('/', schemeEnd); // Find start of path

		if (pathStart != std::string::npos)
		{
			uri = uri.substr(pathStart); // Keep the path
		}
		else
		{
			uri = "/" + uri.substr(schemeEnd); // No path, keep the authority
		}

		// Remove port if any and store in port_
		if (uri.find(':') != std::string::npos)
		{
			size_t		colonPos = uri.find_last_of(':');
			std::string tmpPort = uri.substr(colonPos + 1);
			uri = uri.substr(0, colonPos - 1);
			port_ = std::atol(tmpPort.c_str());
		}
	}

	uri_ = uri;
	host_ = inputHeaders.at("Host")[0];
	target_ = host_ + uri_;
	headers_ = inputHeaders;
	body_ = body;

	// Store body length if present
	// clang-format off
	std::map<std::string, std::vector<std::string> >::const_iterator it
		= headers_.begin();
	// clang-format on
	bodyLength_
		= (it != headers_.end()) ? std::atol(it->second[0].c_str()) : -1;
}
