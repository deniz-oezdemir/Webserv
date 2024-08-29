/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerException.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sebasnadu <johnavar@student.42berlin.de>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/29 11:42:48 by sebasnadu         #+#    #+#             */
/*   Updated: 2024/08/29 13:13:19 by sebasnadu        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <exception>
#include <string>

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
