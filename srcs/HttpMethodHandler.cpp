#include "HttpMethodHandler.hpp"
#include "HttpErrorHandler.hpp"

std::string HttpMethodHandler::handleRequest(
	HttpRequest const &request,
	Server const	  &server,
	std::string const &method
)
{
	if (method == "GET")
		return handleGetRequest_(request, server);
	else if (method == "POST")
		return handlePostRequest_(request, server);
	else if (method == "DELETE")
		return handleDeleteRequest_(request, server);
	return "";
}

std::string HttpMethodHandler::handleGetRequest_(
	const HttpRequest &request,
	Server const	  &server
)
{
	std::string uri = request.getUri();
	bool		keepAlive = request.getKeepAlive();
	// clang-format off
	std::map<std::string, std::vector<std::string> > location;
	// clang-format on

	if (server.isThisLocation(uri))
		location = server.getThisLocation(uri);
	else
		return HttpErrorHandler::getErrorPage(404, keepAlive);

	// Check for redirections
	std::string redirection = handleRedirection_(location, keepAlive);
	if (!redirection.empty())
		return redirection;
	// Check for authorized methods
	if (!validateMethod_(location, "GET"))
		return HttpErrorHandler::getErrorPage(405, keepAlive);
	// Check for max body size
	if (!validateBodySize_(location, request, server))
		return HttpErrorHandler::getErrorPage(413, keepAlive);

	std::string filepath = getFilePath_(uri, location, server);
	std::string rootdir = getRootDir_(location, server);

	if (isCgiRequest_(location, uri))
		return handleCgiRequest_(
			filepath, getCgiInterpreter_(location), request, keepAlive
		);
	// Check if the request is for a directory and handle autoindex
	if (isDirectory_(filepath))
	{
		if (isAutoIndexEnabled_(location))
			return handleAutoIndex_(rootdir, uri, keepAlive);
		// Search for index file in the directory
		filepath = findIndexFile_(filepath, location, server);
	}

	// Check if the file exists and return the response
	return createFileResponse_(filepath, rootdir, server, keepAlive);
}
