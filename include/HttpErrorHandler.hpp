#pragma once

#include "Server.hpp"

#include <string>

class HttpErrorHandler
{
  public:
	static std::string
	getErrorPage(unsigned int const &statusCode, bool const &keepAlive = false);
	static std::string getErrorPage(
		Server const	  &server,
		unsigned int const &statusCode,
		std::string const &rootdir,
		bool const		  &keepAlive
	);

	// handleDefaultErrorResponse_(int errorCode, bool closeConnection = false);
	// static std::string handleServerErrorResponse_(
	// 	Server const	  &server,
	// 	int				   statusCode,
	// 	std::string const &rootdir,
	// 	bool			  &keepAlive
	// );

  private:
	HttpErrorHandler();
	HttpErrorHandler(HttpErrorHandler const &src);
	HttpErrorHandler &operator=(HttpErrorHandler const &src);
	~HttpErrorHandler();
};
