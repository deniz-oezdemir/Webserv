/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migmanu <jmanuelmigoya@gmail.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/28 15:12:01 by migmanu           #+#    #+#             */
/*   Updated: 2024/08/29 19:02:31 by migmanu          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "HttpRequest.hpp"
#include <cstddef>
#include <iostream>

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
