/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migmanu <jmanuelmigoya@gmail.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/28 12:36:58 by migmanu           #+#    #+#             */
/*   Updated: 2024/08/28 19:47:57 by migmanu          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <string>
#include <strstream>
#include <vector>

// Basic class for storing parsed requests
class HttpRequest
{
  public:
	HttpRequest(
		std::string						   &method,
		std::string						   &httpVersion,
		std::string						   &target,
		std::map<std::string, std::string> &headers,
		std::vector<char>				   &body
	);
	HttpRequest(const HttpRequest& src);
	~HttpRequest(void);

	// Setters
	void setMethod(std::string &newMethod);
	void setHttpVersion(std::string &newHttpVersion);
	void setTarget(std::string &newTarget);
	void setHeaders(std::map<std::string, std::string> &newHeaders);
	void setBody(std::vector<char> &newBody);

	// Getters
	const std::string&						   getMethod(void) const;
	const std::string&						   getHttpVersion(void) const;
	const std::string&						   getTarget(void) const;
	const std::map<std::string, std::string>& getHeaders(void) const;
	const std::vector<char>&				   getBody(void) const;

	// Overloaded Operators
	HttpRequest&	 operator=(const HttpRequest& rhs);

  private:
	std::string						   _method;
	std::string						   _httpVersion;
	std::string						   _target;
	std::map<std::string, std::string> _headers;
	std::vector<char>				   _body;
};

std::ostrstream& operator<<(std::ostrstream& os, const HttpRequest& rhs);

