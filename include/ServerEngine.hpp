#pragma once

#include "Client.hpp"
#include "ConfigValue.hpp"
#include "HttpRequest.hpp"
#include "Server.hpp"
#include "macros.hpp"

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
	void start(void);

  private:
	ServerEngine();
	ServerEngine(ServerEngine const &src);
	ServerEngine &operator=(ServerEngine const &src);

	unsigned int		numServers_;
	unsigned int		totalServerInstances_;
	std::vector<pollfd> pollFds_;
	std::vector<Server> servers_;
	size_t				pollIndex_;

	std::vector<Client> clients_;
	long long			clientIndex_;

	void initServer_(
		std::map<std::string, ConfigValue> const &serverConfig,
		size_t									 &serverIndex,
		size_t									 &globalServerIndex
	);
	void		initServerPollFds_(void);
	void		initializePollEvents(void);
	void		processPollEvents(void);
	void		readClientRequest_(size_t &index);
	void		processClientRequest_(size_t &index);
	std::string createResponse(const HttpRequest &request);
	void		sendResponse_(size_t &index, const std::string &response);
	bool		isPollFdServer_(int &fd);
	void		acceptConnection_(size_t &index);
	void		restartServer_(size_t &index);
	void		pollFdError_(size_t &index);
	void		closeConnection_(size_t &index);

	int findServer_(std::string const &host, unsigned short const &port);
};
