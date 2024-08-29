#include "../include/HttpRequest.hpp"
#include <sstream>

HttpRequest::HttpRequest(
	std::string&						method,
	std::string&						httpVersion,
	std::string&						target,
	std::map<std::string, std::string>& headers,
	std::vector<char>&					body
)
	: _method(method), _httpVersion(httpVersion), _target(target),
	  _headers(headers), _body(body)
{
	return;
}

HttpRequest::HttpRequest(const HttpRequest& src)
{
	_method = src._method;
	_httpVersion = src._httpVersion;
	_target = src._target;
	_headers = src._headers;
	_body = src._body;

	return;
}

HttpRequest::~HttpRequest(void)
{
	return;
}

void HttpRequest::setMethod(std::string &newMethod)
{
	_method = newMethod;
}

void HttpRequest::setHttpVersion(std::string &newHttpVersion)
{
	_httpVersion = newHttpVersion;
}

void HttpRequest::setTarget(std::string &newTarget)
{
	_target = newTarget;
}

void HttpRequest::setHeaders(std::map<std::string, std::string> &newHeaders)
{
	_headers = newHeaders;
}

void HttpRequest::setBody(std::vector<char> &newBody)
{
	_body = newBody;
}

const std::string& HttpRequest::getMethod(void) const
{
	return _method;
}

const std::string& HttpRequest::getHttpVersion(void) const
{
	return _httpVersion;
}

const std::string& HttpRequest::getTarget(void) const
{
	return _target;
}

const std::map<std::string, std::string>& HttpRequest::getHeaders(void) const
{
	return _headers;
}

const std::vector<char>& HttpRequest::getBody(void) const
{
	return _body;
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
