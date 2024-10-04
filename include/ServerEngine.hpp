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

/**
 * @class ServerEngine
 * @brief Manages server operations, including client connections and HTTP
 * requests.
 *
 * The ServerEngine class initializes server instances, manages client
 * connections, and processes HTTP requests using the poll system call to
 * monitor file descriptors.
 *
 * It maintains vectors of poll file descriptors and client instances, and
 * provides methods to initialize servers, process poll events, and handle
 * client requests and responses.
 *
 * @note The class uses two indices: pollIndex_ to track the current position
 * in the pollFds_ vector and clientIndex_ to track the current position in
 * the clients_ vector. Since the pollFds_ vector contains poll file
 * descriptors for both servers and clients, clientIndex_ is always behind
 * pollIndex_ by the total number of server instances.
 */

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
	std::vector<Client> clients_;
	size_t				pollIndex_;
	long long			clientIndex_;

	void initServer_(
		std::map<std::string, ConfigValue> const &serverConfig,
		size_t									 &serverIndex,
		size_t									 &globalServerIndex
	);
	void		initServerPollFds_(void);
	void		initializePollEvents(void);
	void		processPollEvents(void);
	void		readClientRequest_(size_t &pollIndex_);
	void		processClientRequest_(size_t &pollIndex_);
	std::string createResponse(const HttpRequest &request);
	void		sendResponse_(size_t &pollIndex_, const std::string &response);
	bool		isPollFdServer_(int &fd);
	void		acceptConnection_(size_t &pollIndex_);
	void		restartServer_(size_t &pollIndex_);
	void		pollFdError_(size_t &pollIndex_);
	void		closeConnection_(size_t &pollIndex_);

	int findServer_(std::string const &host, unsigned short const &port);
};
