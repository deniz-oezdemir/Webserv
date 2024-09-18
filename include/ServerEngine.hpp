#pragma once

#include "ConfigValue.hpp"
#include "HttpRequest.hpp"
#include "Server.hpp"
#include <cstring>
#include <map>
#include <poll.h>
#include <string>

class ServerEngine
{
  public:
	ServerEngine(std::vector<std::map<std::string, ConfigValue> > const &servers
	);
	~ServerEngine();

	void start(void);

  private:
	ServerEngine();
	ServerEngine(ServerEngine const &src);
	ServerEngine &operator=(ServerEngine const &src);

	unsigned int		numServers_;
	unsigned int		totalServerInstances_;
	std::vector<pollfd> pollFds_;
	std::vector<Server> servers_;

	void initServer_(
		std::map<std::string, ConfigValue> const &serverConfig,
		size_t									 &serverIndex,
		size_t									 &globalServerIndex
	);
	void initPollFds_(void);
	bool isPollFdServer_(int &fd);
	void handleClient_(size_t &index);
	void acceptConnection_(size_t &index);
	void restartServer_(size_t &index);
	void pollFdError_(size_t &index);

	std::string createResponse(const HttpRequest &request);
	std::string handleGetRequest(const HttpRequest &request);
	std::string handlePostRequest(const HttpRequest &request);
	std::string handleDeleteRequest(const HttpRequest &request);
	std::string handleNotImplementedRequest();

	std::string createTimestamp();
};
