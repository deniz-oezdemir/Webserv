#include "../include/HttpRequest.hpp"
#include <ostream>

HttpRequest::HttpRequest(
	std::string&						method,
	std::string&						httpVersion,
	std::string&						target,
	std::map<std::string, std::string>& headers,
	std::vector<char>&					body
)
	: method_(method), httpVersion_(httpVersion), target_(target),
	  headers_(headers), body_(body)
{
	return;
}

HttpRequest::HttpRequest(const HttpRequest& src)
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

void HttpRequest::setHeaders(std::map<std::string, std::string> &newHeaders)
{
	headers_ = newHeaders;
}

void HttpRequest::setBody(std::vector<char> &newBody)
{
	body_ = newBody;
}

const std::string& HttpRequest::getMethod(void) const
{
	return method_;
}

const std::string& HttpRequest::getHttpVersion(void) const
{
	return httpVersion_;
}

const std::string& HttpRequest::getTarget(void) const
{
	return target_;
}

const std::map<std::string, std::string>& HttpRequest::getHeaders(void) const
{
	return headers_;
}

const std::vector<char>& HttpRequest::getBody(void) const
{
	return body_;
}

std::ostream& operator<<(std::ostream& os, const HttpRequest& rhs)
{
    os << "Method: " << rhs.getMethod() << std::endl;
    os << "HTTP version: " << rhs.getHttpVersion() << std::endl;
    os << "Target: " << rhs.getTarget() << std::endl;
	os << "HEADERS:" << std::endl;
    const std::map<std::string, std::string>& headersCpy = rhs.getHeaders();
    for (std::map<std::string, std::string>::const_iterator it = headersCpy.begin();
         it != headersCpy.end();
         it++)
    {
        os << it->first << " : " << it->second << std::endl;
    }
	os << "BODY:" << std::endl;
    const std::vector<char>& bodyCpy = rhs.getBody();
    for (std::vector<char>::const_iterator it = bodyCpy.begin(); it != bodyCpy.end();
         it++)
    {
        os << *it << std::endl;
    }

    return os;
}
