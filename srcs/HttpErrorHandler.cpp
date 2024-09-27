#include "HttpErrorHandler.hpp"
#include "HttpResponse.hpp"
#include "Logger.hpp"
#include "macros.hpp"
#include "utils.hpp"

std::string HttpErrorHandler::getErrorPage(
	Server const	   &server,
	unsigned int const &statusCode,
	std::string const  &rootdir,
	bool const		   &keepAlive
)
{
	HttpResponse response;
	std::string	 errorURI;

	if (server.getErrorPageValue(statusCode, errorURI))
	{
		response.setStatusCode(statusCode);
		response.setReasonPhrase(ft::getStatusCodeReason(statusCode));
		response.setHeader("Server", SERVER_NAME);
		response.setHeader("Date", ft::createTimestamp());
		response.setHeader("Content-Type", "text/html; charset=UTF-8");

		std::string body = ft::readFile(rootdir + errorURI);

		response.setHeader("Content-Length", std::to_string(body.size()));
		if (keepAlive)
			response.setHeader("Connection", "keep-alive");
		else
			response.setHeader("Connection", "close");
		response.setBody(body);

		return response.toString();
	}
	return "";
}

std::string HttpErrorHandler::getErrorPage(
	unsigned int const &statusCode,
	bool const		   &keepAlive
)
{
	Logger::log(Logger::DEBUG)
		<< "Handling default error response: [" << statusCode << "] "
		<< ft::getStatusCodeReason(statusCode) << std::endl;

	HttpResponse response;
	response.setStatusCode(statusCode);
	response.setReasonPhrase(ft::getStatusCodeReason(statusCode));
	response.setHeader("Server", SERVER_NAME);
	response.setHeader("Date", ft::createTimestamp());
	response.setHeader("Content-Type", "text/html; charset=UTF-8");

	std::string body
		= ft::readErrorPage("./www/" + std::to_string(statusCode) + ".html");
	if (body.empty())
	{
		if (statusCode >= 400 && statusCode < 500)
			body = ft::readErrorPage("./www/4xx.html");
		else if (statusCode >= 500 && statusCode < 600)
			body = ft::readErrorPage("./www/5xx.html");
		else if (statusCode >= 300 && statusCode < 400)
			body = ft::readErrorPage("./www/3xx.html");
	}
	if (body.empty())
		body = "<!DOCTYPE html>\n<html>\n<head><title>"
			   + std::to_string(statusCode) + " "
			   + ft::getStatusCodeReason(statusCode)
			   + "</title></head>\n<body><h1>" + std::to_string(statusCode)
			   + " " + ft::getStatusCodeReason(statusCode)
			   + "</h1></body>\n</html>\n";

	response.setHeader("Content-Length", std::to_string(body.size()));
	if (keepAlive)
		response.setHeader("Connection", "keep-alive");
	else
		response.setHeader("Connection", "close");
	response.setBody(body);

	return response.toString();
}
