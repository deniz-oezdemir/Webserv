/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ARequestParser.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migmanu <jmanuelmigoya@gmail.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/28 15:12:01 by migmanu           #+#    #+#             */
/*   Updated: 2024/08/28 15:21:57 by migmanu          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "HttpRequest.hpp"

class RequestParser
{
  public:
	virtual ~RequestParser(void) = 0;

	virtual HttpRequest parseRequest(std::string request) = 0;

  private:
	RequestParser(void);
	RequestParser(const RequestParser &src);
	RequestParser &operator=(const RequestParser &rhs);
};
