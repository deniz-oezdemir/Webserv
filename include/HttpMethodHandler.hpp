#pragma once

#include "HttpRequest.hpp"
#include "Server.hpp"

#include <map>
#include <string>

class HttpMethodHandler
{
  public:
	static std::string handleRequest(
		const HttpRequest &request,
		Server const	  &server,
		std::string const &method
	);

  private:
	HttpMethodHandler();
	HttpMethodHandler(HttpMethodHandler const &src);
	~HttpMethodHandler();
	HttpMethodHandler &operator=(HttpMethodHandler const &src);

	static std::string
	generateAutoIndexPage_(std::string const &root, std::string const &uri);

	// clang-format off
	static bool validateMethod_(
		std::map<std::string, std::vector<std::string> > const &location,
		std::string const									  &method
	);
	static bool validateBodySize_(
		std::map<std::string, std::vector<std::string> > const &location,
		HttpRequest const									  &request,
		Server const										  &server
	);
	static std::string getFilePath_(
		std::string const									  &uri,
		std::map<std::string, std::vector<std::string> > const &location,
		Server const										  &server
	);
	static bool isCgiRequest_(
		std::map<std::string, std::vector<std::string> > const &location,
		std::string const									  &uri
	);
	static std::string getCgiInterpreter_(
		std::map<std::string, std::vector<std::string> > const &location
	);
	static bool isAutoIndexEnabled_(
		std::map<std::string, std::vector<std::string> > const &location
	);
	static std::string findIndexFile_(
		const std::string									  &filepath,
		const std::map<std::string, std::vector<std::string> > &location,
		const Server										  &server
	);
	static bool isDirectory_(std::string const &filepath);
	static std::string getRootDir_(
		std::map<std::string, std::vector<std::string> > const &location,
		Server const										  &server
	);
	// clang-format on

	static std::string createFileResponse_(
		std::string const &filepath,
		std::string const &rootdir,
		Server const	  &server,
		bool			  &keepAlive
	);

	static std::string handleRedirection_(
		std::map<std::string, std::vector<std::string> > const &location,
		bool												  &keepAlive
	);
	static std::string handleReturnDirective_(
		std::vector<std::string> const &returnDirective,
		bool						   &keepAlive
	);
	static std::string handleAutoIndex_(
		std::string const &root,
		std::string const &uri,
		bool			  &keepAlive
	);
	static std::string handleCgiRequest_(
		std::string const &filepath,
		std::string const &interpreter,
		HttpRequest const &request,
		bool			  &keepAlive
	);

	static std::string
	handleGetRequest_(const HttpRequest &request, Server const &server);
	static std::string
	handlePostRequest_(const HttpRequest &request, Server const &server);
	static std::string
	handleDeleteRequest_(const HttpRequest &request, Server const &server);
};
