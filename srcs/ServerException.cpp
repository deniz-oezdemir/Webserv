/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerException.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sebasnadu <johnavar@student.42berlin.de>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/29 11:51:32 by sebasnadu         #+#    #+#             */
/*   Updated: 2024/08/29 13:32:08 by sebasnadu        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerException.hpp"
#include "utils.hpp"

ServerException::ServerException(
	std::string const& msg,
	int				   err_num,
	std::string const& arg
)
	: _errno(err_num), _msg(msg)
{
	if (this->_msg.find('%') != std::string::npos)
		this->_msg.replace(this->_msg.find('%'), 1, arg);
	if (this->_errno)
		this->_msg += " : (" + ft::to_string(this->_errno) + ") " + strerror(this->_errno);
}

char const* ServerException::what(void) const throw()
{
	return this->_msg.c_str();
}

ServerException::ServerException(ServerException const& src)
{
	*this = src;
}

ServerException	&ServerException::operator=(ServerException const &src)
{
	if (this != &src)
	{
		this->_errno = src._errno;
		this->_msg = src._msg;
	}
  return *this;
}

ServerException::~ServerException() throw() {}
