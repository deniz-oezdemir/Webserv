#include "../include/HttpException.hpp"

HttpException::HttpException() : _code(0), _message("") {}

HttpException::HttpException(int _code, const std::string &_message)
	: _code(_code), _message(_message)
{
}

HttpException::HttpException(const HttpException &other)
	: _code(other._code), _message(other._message)
{
}

HttpException &HttpException::operator=(const HttpException &other)
{
	if (this != &other)
	{
		_code = other._code;
		_message = other._message;
	}
	return *this;
}

HttpException::~HttpException() throw() {}

int HttpException::getCode() const
{
	return _code;
}

const char *HttpException::what() const throw()
{
	return _message.c_str();
}
