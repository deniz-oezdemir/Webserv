#pragma once

#include "ConfigValue.hpp"
#include "HttpRequest.hpp"
#include "Server.hpp"
#include "macros.hpp"
#include <cstring>
#include <map>
#include <poll.h>
#include <string>

extern bool g_shutdown;

class ServerEngine
{
  public:
	// clang-format off
	ServerEngine(std::vector<std::map<std::string, ConfigValue> > const &servers
	);
	// clang-format on
	~ServerEngine();

	void start(void);

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

	std::string handleGetRequest_(const HttpRequest &request);
	std::string handlePostRequest_(const HttpRequest &request);
	std::string handleDeleteRequest_(const HttpRequest &request);
	std::string handleNotImplementedRequest_();

	std::string createTimestamp_();
	std::string readFile_(const std::string &filePath);
};
