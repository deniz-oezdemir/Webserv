/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpRequest.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: migmanu <jmanuelmigoya@gmail.com>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/28 12:36:58 by migmanu           #+#    #+#             */
/*   Updated: 2024/08/29 12:12:34 by migmanu          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <string>
<<<<<<< HEAD
#include <strstream>
#include <vector>

/*
 * The HttpRequest class encapsulates an HTTP request, providing methods to
 * manipulate and retrieve its components (method, HTTP version, target, headers,
 * and body). It also overloads the << operator for std::ostrstream output.
 */
=======
#include <vector>

>>>>>>> 1c47667 (feat(HttpRequest): add HttpRequest class and HTTP macros)
class HttpRequest
{
  public:
	HttpRequest(
<<<<<<< HEAD
		std::string						   &method,
		std::string						   &httpVersion,
		std::string						   &target,
		std::map<std::string, std::string> &headers,
		std::vector<char>				   &body
=======
		std::string						   _method,
		std::map<std::string, std::string> _headers,
		std::string						   _target,
		std::string						   _httpVersion,
		std::vector<char>				   _body
>>>>>>> 1c47667 (feat(HttpRequest): add HttpRequest class and HTTP macros)
	);
	HttpRequest(const HttpRequest& src);
	~HttpRequest(void);

	// Setters
<<<<<<< HEAD
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

=======
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
>>>>>>> 1c47667 (feat(HttpRequest): add HttpRequest class and HTTP macros)
