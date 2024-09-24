#pragma once

#include "ConfigValue.hpp"
#include "HttpRequest.hpp"
#include "Server.hpp"
#include "macros.hpp"
#include <cstring>
#include <map>
#include <poll.h>
#include <string>
#include <sys/wait.h>

extern bool g_shutdown;

class ServerEngine
{
  public:
	// clang-format off
	ServerEngine(std::vector<std::map<std::string, ConfigValue> > const &servers
	);
	// clang-format on
	~ServerEngine();

	void			   start(void);
	std::string const &getStatusCodeReason(unsigned int statusCode) const;
	std::string		   getMimeType(std::string const &filePath) const;

	// TODO: move createResponse() to private as only public for testing
	std::string createResponse(const HttpRequest &request);

  private:
	ServerEngine();
	ServerEngine(ServerEngine const &src);
	ServerEngine &operator=(ServerEngine const &src);

	unsigned int		numServers_;
	unsigned int		totalServerInstances_;
	std::vector<pollfd> pollFds_;
	std::vector<Server> servers_;

	char clientRequestBuffer_[BUFFER_SIZE];
	long bytesRead_;

	void initServer_(
		std::map<std::string, ConfigValue> const &serverConfig,
		size_t									 &serverIndex,
		size_t									 &globalServerIndex
	);
	void initServerPollFds_(void);
	void initializePollEvents();
	void processPollEvents();
	void readClientRequest_(size_t &index);
	void sendClientResponse_(size_t &index);
	bool isPollFdServer_(int &fd);
	void acceptConnection_(size_t &index);
	void restartServer_(size_t &index);
	void pollFdError_(size_t &index);

	int findServer_(std::string const &host, unsigned short const &port);
	std::string
	generateAutoIndexPage_(std::string const &root, std::string const &uri);
	// clang-format off
	std::string handleRedirection_(
		std::map<std::string, std::vector<std::string> > const &location,
		bool												  &keepAlive
	);
	bool validateMethod_(
		std::map<std::string, std::vector<std::string> > const &location,
		std::string const									  &method
	);
	bool validateBodySize_(
		std::map<std::string, std::vector<std::string> > const &location,
		HttpRequest const									  &request,
		Server const										  &server
	);
	std::string getFilePath_(
		std::string const									  &uri,
		std::map<std::string, std::vector<std::string> > const &location,
		Server const										  &server
	);
	bool isCgiRequest_(
		std::map<std::string, std::vector<std::string> > const &location,
		std::string const									  &uri
	);
	std::string getCgiInterpreter_(
		std::map<std::string, std::vector<std::string> > const &location
	);

	bool isAutoIndexEnabled_(
		std::map<std::string, std::vector<std::string> > const &location
	);

	std::string findIndexFile_(
		const std::string									  &filepath,
		const std::map<std::string, std::vector<std::string> > &location,
		const Server										  &server
	);

	bool isDirectory_(std::string const &filepath);

	std::string getRootDir_(
		std::map<std::string, std::vector<std::string> > const &location,
		Server const										  &server
	);

	std::string createFileResponse_(
		std::string const &filepath,
		std::string const &rootdir,
		Server const	  &server,
		bool			  &keepAlive
	);
	// clang-format on

	std::string handleReturnDirective_(
		std::vector<std::string> const &returnDirective,
		bool						   &keepAlive
	);
	std::string handleAutoIndex_(
		std::string const &root,
		std::string const &uri,
		bool			  &keepAlive
	);
	std::string handleCgiRequest_(
		std::string const &filepath,
		std::string const &interpreter,
		HttpRequest const &request,
		bool			  &keepAlive
	);

	std::string
	handleGetRequest_(const HttpRequest &request, Server const &server);
	std::string
	handlePostRequest_(const HttpRequest &request, Server const &server);
	std::string
	handleDeleteRequest_(const HttpRequest &request, Server const &server);
	std::string
	handleDefaultErrorResponse_(int errorCode, bool closeConnection = false);
	std::string handleServerErrorResponse_(
		Server const	  &server,
		int				   statusCode,
		std::string const &rootdir,
		bool			  &keepAlive
	);

	std::string createTimestamp_();
};
