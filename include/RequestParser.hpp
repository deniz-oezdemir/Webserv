/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migmanu <jmanuelmigoya@gmail.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/28 15:12:01 by migmanu           #+#    #+#             */
<<<<<<< HEAD
/*   Updated: 2024/08/29 12:13:40 by migmanu          ###   ########.fr       */
=======
/*   Updated: 2024/08/28 18:54:54 by migmanu          ###   ########.fr       */
>>>>>>> d1a698e (feat: refactor HttpRequest and related classes for C++98)
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "HttpRequest.hpp"
#include <cstddef>

class RequestParser
{
  public:
	static HttpRequest parseRequest(std::string str);

  private:
	RequestParser(void);
	~RequestParser(void);
	RequestParser(const RequestParser &src);
	RequestParser &operator=(const RequestParser &rhs);
};
