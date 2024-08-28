/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestParser.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migmanu <jmanuelmigoya@gmail.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/28 15:33:11 by migmanu           #+#    #+#             */
/*   Updated: 2024/08/28 18:57:37 by migmanu          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/RequestParser.hpp"

HttpRequest RequestParser::parseRequest(std::string str)
{
	std::string						   method;
	std::string						   httpVersion;
	std::string						   target;
	std::map<std::string, std::string> headers;
	std::vector<char>				   body;

	std::size_t pos = str.find_first_of("\r\n");
	std::string firstline(str.begin(), str.begin() + pos);
	str.erase(str.begin(), str.begin() + pos);

	HttpRequest req(method, httpVersion, target, headers, body);

	return req;
}
