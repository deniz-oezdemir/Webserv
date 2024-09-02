#include "ServerException.hpp"
#include "utils.hpp"

// Constructor for the ServerException class. It replaces the % if it is found
// in the message with the arg string. if err_num is not 0, it appends the errno
// and the strerror(errno) to the message.
ServerException::ServerException(
	std::string const &msg,
	int				   err_num,
	std::string const &arg
)
	: _msg(msg)
{
	if (this->_msg.find('%') != std::string::npos)
		this->_msg.replace(this->_msg.find('%'), 1, arg);
	if (err_num != 0)
		this->_msg +=
			" : (" + ft::toString(err_num) + ") " + strerror(err_num);
}

char const *ServerException::what(void) const throw()
{
	return this->_msg.c_str();
}

ServerException::ServerException(ServerException const &src)
{
	*this = src;
}

ServerException &ServerException::operator=(ServerException const &src)
{
	if (this != &src)
		this->_msg = src._msg;
	return *this;
}

ServerException::~ServerException() throw() {}
