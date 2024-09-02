#pragma once

#include <exception>
#include <string>
#include <cstring>

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

	int					_errno;
	std::string	_msg;
};
