#include "../include/HttpException.hpp"

HttpException::HttpException() : code_(0), message_("") {}

HttpException::HttpException(int code_, const std::string &message_)
	: code_(code_), message_(message_)
{
}

HttpException::HttpException(const HttpException &other)
	: code_(other.code_), message_(other.message_)
{
}

HttpException &HttpException::operator=(const HttpException &other)
{
	if (this != &other)
	{
		code_ = other.code_;
		message_ = other.message_;
	}
	return *this;
}

HttpException::~HttpException() throw() {}

int HttpException::getCode() const
{
	return code_;
}

const char *HttpException::what() const throw()
{
	return message_.c_str();
}
