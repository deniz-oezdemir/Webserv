#include "../include/HttpResponse.hpp"
#include "utils.hpp"

// Might have to change default values
HttpResponse::HttpResponse() : statusCode_(200), reasonPhrase_("OK") {}

void HttpResponse::setBody(const std::string &body)
{
	body_ = body;
}

HttpResponse::~HttpResponse() {}

void HttpResponse::setStatusCode(int code)
{
	statusCode_ = code;
}

void HttpResponse::setReasonPhrase(const std::string &phrase)
{
	reasonPhrase_ = phrase;
}

void HttpResponse::setHeader(const std::string &key, const std::string &value)
{
	headers_.push_back(std::make_pair(key, value));

	// map
	// headers_[key] = value;
}

std::string HttpResponse::toString() const
{
	std::string response;

	// Start line
	response += "HTTP/1.1 " + ft::toString(statusCode_) + " " + reasonPhrase_
				+ "\r\n";

	// Headers
	std::vector<std::pair<std::string, std::string> >::const_iterator it;
	for (it = headers_.begin(); it != headers_.end(); ++it)
	{
		response += it->first + ": " + it->second + "\r\n";
	}

	// Blank line
	response += "\r\n";

	// Body
	response += body_;

	return response;
}

std::string const &HttpResponse::getHeader(std::string const &key) const
{
	std::vector<std::pair<std::string, std::string> >::const_iterator it;
	for (it = headers_.begin(); it != headers_.end(); ++it)
	{
		if (it->first == key)
			return it->second;
	}
	return headers_.end()->second;
}
