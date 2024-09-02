#pragma once

#include <exception>
#include <string>
#include <cstring>

// ServerException is a custom exception class for the server.
class ServerException : virtual public std::exception
{
public:
	ServerException(
		std::string const &msg,
		int				   err_num = 0,
		std::string const &arg = ""
	);
	ServerException(ServerException const &src);
	virtual ~ServerException() throw();

	ServerException &operator=(ServerException const &src);

	char const *what() const throw();

private:
	ServerException();

	std::string	_msg;
};
