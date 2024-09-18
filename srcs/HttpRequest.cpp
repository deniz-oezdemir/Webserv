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
