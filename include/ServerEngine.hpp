#pragma once

#include "ConfigValue.hpp"
#include "HttpRequest.hpp"
#include "Server.hpp"
#include "macros.hpp"
#include <algorithm>
#include <cstddef>
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
	// TODO: introduce pollIndex_ such that we do not have to pass it as arg

	// TODO: combine into one vector of triplets to improve cache usage
	std::vector<std::stringstream> clientRequestBuffer_;
	std::vector<std::string>	   clientReadFullRequest_;
	std::vector<long long>		   totalBytesRead_;
	std::vector<bool>			   isDoneReading_;
	size_t						   clientIndex_;

	void initServer_(
		std::map<std::string, ConfigValue> const &serverConfig,
		size_t									 &serverIndex,
		size_t									 &globalServerIndex
	);
	void initServerPollFds_(void);
	void initializePollEvents(void);
	void processPollEvents(void);
	void readClientRequest_(size_t &index);
	void sendClientResponse_(size_t &index);
	bool isPollFdServer_(int &fd);
	void acceptConnection_(size_t &index);
	void restartServer_(size_t &index);
	void pollFdError_(size_t &index);

	int findServer_(std::string const &host, unsigned short const &port);
	std::string
	generateAutoIndexPage_(std::string const &root, std::string const &uri);

	std::string
	handleReturnDirective_(std::vector<std::string> const &returnDirective);
	std::string
	handleAutoIndex_(std::string const &root, std::string const &uri);
	std::string handleCgiRequest_(
		std::string const &filepath,
		std::string const &interpreter,
		HttpRequest const &request
	);

	std::string
	handleGetRequest_(const HttpRequest &request, Server const &server);
	std::string
	handlePostRequest_(const HttpRequest &request, Server const &server);
	std::string
	handleDeleteRequest_(const HttpRequest &request, Server const &server);
	std::string handleNotImplementedRequest_();
	std::string
	handleDefaultErrorResponse_(int errorCode, bool closeConnection = false);

	// TODO: move to utils
	std::string createTimestamp_();
};
