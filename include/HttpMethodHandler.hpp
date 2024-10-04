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

	static std::string generateAutoIndexPage_(
		std::string const &root,
		std::string const &uri,
		Server const	  &server,
		bool const		  &keepAlive
	);

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
	static std::string getUploadPath_(
		std::map<std::string, std::vector<std::string> > const &location
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

	static std::string createFileGetResponse_(
		std::string const &filepath,
		std::string const &rootdir,
		Server const	  &server,
		bool const		  &keepAlive
	);

	static std::string createFilePostResponse_(
		HttpRequest const &request,
		const std::string &rootdir,
		std::string const &redirect,
		std::string const &uploadpath,
		Server const	  &server,
		bool const		  &keepAlive
	);

	static std::string createDeleteResponse_(
		HttpRequest const &request,
		const std::string &filepath,
		const std::string &rootdir,
		std::string const &redirect,
		const Server	  &server,
		bool			   keepAlive
	);

	// clang-format off
	static std::string handleRedirection_(
		std::map<std::string, std::vector<std::string> > const &location,
		bool const											  &keepAlive
	); // clang-format on
	static std::string handleReturnDirective_(
		std::vector<std::string> const &returnDirective,
		bool const					   &keepAlive
	);
	static std::string handleAutoIndex_(
		std::string const &root,
		std::string const &uri,
		Server const	  &server,
		bool const		  &keepAlive
	);
	static std::string handleCgiRequest_(
		std::string const &filepath,
		std::string const &interpreter,
		HttpRequest const &request,
		bool const		  &keepAlive,
		Server const	  &server,
		std::string const &rootdir,
		std::string const &redirect = "",
		std::string const &uploadpath = ""
	);

	static std::string handleErrorResponse_(
		Server const	  &server,
		int const		  &errorCode,
		std::string const &rootdir,
		bool const		  &keepAlive
	);

	static std::string
	handleGetRequest_(const HttpRequest &request, Server const &server);
	static std::string
	handlePostRequest_(const HttpRequest &request, Server const &server);
	static std::string
	handleDeleteRequest_(const HttpRequest &request, Server const &server);
};
