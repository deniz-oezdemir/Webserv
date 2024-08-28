/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migmanu <jmanuelmigoya@gmail.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/28 12:36:58 by migmanu           #+#    #+#             */
/*   Updated: 2024/08/28 13:35:23 by migmanu          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <string>
#include <vector>

class HttpRequest
{
  public:
	HttpRequest(
		std::string						   _method,
		std::map<std::string, std::string> _headers,
		std::string						   _target,
		std::string						   _httpVersion,
		std::vector<char>				   _body
	);
	HttpRequest(const HttpRequest& src);
	~HttpRequest(void);

	// Setters
	void setMethod(std::string newMethod);
	void setHeaders(std::map<std::string, std::string> newHeaders);
	void setTarget(std::string newTarget);
	void setHttpVersion(std::string newHttpVersion);
	void setBody(std::vector<char> newBody);

	// Getters
	void getMethod(void);
	void getHeaders(void);
	void getTarget(void);
	void getHttpVersion(void);
	void getBody(void);

	// Overloaded Operators
	HttpRequest& operator=(const HttpRequest& rhs);

  private:
	std::string						   _method;
	std::map<std::string, std::string> _headers;
	std::string						   _target;
	std::string						   _httpVersion;
	std::vector<char>				   _body;
};
