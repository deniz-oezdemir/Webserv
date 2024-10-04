#include "ServerEngine.hpp"
#include "HttpErrorHandler.hpp"
#include "HttpMethodHandler.hpp"
#include "Logger.hpp"
#include "request_parser/RequestParser.hpp"
#include "utils.hpp"

#include <csignal>
#include <dirent.h>
#include <errno.h>
#include <exception>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

/**
 * @brief Constructor for ServerEngine.
 *
 * Initializes the ServerEngine with the given server configurations.
 *
 * @param servers A vector of maps containing server configurations.
 */
ServerEngine::ServerEngine(
	// clang-format off
	std::vector<std::map<std::string, ConfigValue> > const &servers
	// clang-format on
)
	: numServers_(servers.size())
{
	Logger::log(Logger::INFO)
		<< "Initializing the Server Engine with " << this->numServers_
		<< " servers..." << std::endl;
	size_t globalServerIndex(0);
	for (size_t serverIndex = 0; serverIndex < this->numServers_; ++serverIndex)
	{
		globalServerIndex
			+= servers[serverIndex].at("listen").getMapValue("port").size();
	}
	this->servers_.reserve(globalServerIndex);
	globalServerIndex = 0;
	for (size_t serverIndex = 0; serverIndex < this->numServers_; ++serverIndex)
	{
		this->initServer_(servers[serverIndex], serverIndex, globalServerIndex);
	}
	this->totalServerInstances_ = globalServerIndex;

	pollIndex_ = 0;
	clients_.clear();
	clientIndex_ = 0;
}

/**
 * @brief Destructor for ServerEngine.
 *
 * Shuts down the server engine and closes all open file descriptors.
 */
ServerEngine::~ServerEngine()
{
	Logger::log(Logger::DEBUG)
		<< "Shutting down the server engine" << std::endl;
	for (size_t i = 0; i < pollFds_.size(); ++i)
	{
		if (!this->isPollFdServer_(pollFds_[i].fd) && pollFds_[i].fd != -1)
		{
			close(pollFds_[i].fd);
			pollFds_[i].fd = -1;
		}
	}
}

/**
 * @brief Initializes a server instance.
 *
 * @param serverConfig The configuration map for the server.
 * @param serverIndex The index of the server in the configuration vector.
 * @param globalServerIndex The global index of the server instance.
 */
void ServerEngine::initServer_(
	std::map<std::string, ConfigValue> const &serverConfig,
	size_t									 &serverIndex,
	size_t									 &globalServerIndex
)
{
	size_t listenSize = serverConfig.at("listen").getMapValue("port").size();
	for (size_t listenIndex = 0; listenIndex < listenSize; ++listenIndex)
	{
		this->servers_.push_back(
			Server(serverConfig, globalServerIndex, listenIndex)
		);
		this->servers_[globalServerIndex].init();
		Logger::log(Logger::INFO)
			<< "Server " << serverIndex + 1 << "[" << listenIndex << "] "
			<< " fd: " << servers_[globalServerIndex].getServerFd() << "| "
			<< "Listen: " << this->servers_[globalServerIndex].getIPV4() << ':'
			<< this->servers_[globalServerIndex].getPort() << std::endl;
		++globalServerIndex;
	}
}

/**
 * @brief Restarts a server instance.
 *
 * Closes and reinitializes the server file descriptor, then updates the
 * pollFds_ vector.
 *
 * @param index The index of the server to restart.
 */
void ServerEngine::restartServer_(size_t &index)
{
	Logger::log(Logger::INFO)
		<< "Restarting server[" << index << "]" << std::endl;
	this->servers_[index].resetServer();
	pollfd serverPollFd = {servers_[index].getServerFd(), POLLIN, 0};
	pollFds_[index] = serverPollFd;
	Logger::log(Logger::INFO)
		<< "Server[" << index << "] restarted" << std::endl;
}

/**
 * @brief Initializes the pollFds_ vector with server file descriptors.
 */
void ServerEngine::initServerPollFds_(void)
{
	pollFds_.clear();
	pollFds_.reserve(this->totalServerInstances_);

	Logger::log(Logger::DEBUG)
		<< "Initializing pollFds_ vector with ServerFds" << std::endl;

	for (size_t i = 0; i < this->totalServerInstances_; ++i)
	{
		pollfd serverPollFd = {servers_[i].getServerFd(), POLLIN, 0};
		pollFds_.push_back(serverPollFd);
	}
}

/**
 * @brief Checks if a file descriptor belongs to a server.
 *
 * @param fd The file descriptor to check.
 * @return true if the file descriptor belongs to a server, false otherwise.
 */
bool ServerEngine::isPollFdServer_(int &fd)
{
	for (size_t i = 0; i < this->totalServerInstances_; ++i)
	{
		if (fd == this->servers_[i].getServerFd())
			return true;
	}
	return false;
}

/**
 * @brief Accepts a new client connection.
 *
 * @param index The index of the server in the pollFds_ vector.
 */
void ServerEngine::acceptConnection_(size_t &index)
{
	Logger::log(Logger::DEBUG) << "Accepting client connection on the server["
							   << index << ']' << std::endl;
	sockaddr_in serverAddr = this->servers_[index].getServerAddr();
	int			addrLen = sizeof(serverAddr);
	int			clientFd = accept(
		this->servers_[index].getServerFd(),
		(struct sockaddr *)&serverAddr,
		(socklen_t *)&addrLen
	);
	if (clientFd < 0)
	{
		if (errno != EWOULDBLOCK && errno != EAGAIN)
		{
			Logger::log(Logger::ERROR, true)
				<< "Failed to accept client connection: ("
				<< ft::toString(errno) << ") " << strerror(errno) << std::endl;
		}
		return;
	}
	Logger::log(Logger::DEBUG) << "Client connection accepted" << std::endl;

	int flags = fcntl(clientFd, F_GETFL, 0);
	if (flags == -1)
	{
		Logger::log(Logger::ERROR, true)
			<< "Failed to get client socket flags: (" << ft::toString(errno)
			<< ") " << strerror(errno) << std::endl;
		close(clientFd);
		return;
	}
	if (fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		Logger::log(Logger::ERROR, true)
			<< "Failed to set client socket flags: (" << ft::toString(errno)
			<< ") " << strerror(errno) << std::endl;
		close(clientFd);
		return;
	}
	Logger::log(Logger::DEBUG)
		<< "Client socket set to non-blocking mode" << std::endl;

	pollfd clientPollFd = {clientFd, POLLIN, 0};
	pollFds_.push_back(clientPollFd);
	Logger::log(Logger::DEBUG)
		<< "Client connection added to pollFds_[" << index << "]" << std::endl;

	Client client(clientPollFd.fd);
	clients_.push_back(client);
	Logger::log(Logger::DEBUG)
		<< "Client added to clients_[" << clientIndex_ << "]" << std::endl;
}

/**
 * @brief Handles errors on a poll file descriptor.
 *
 * Logs the error and closes the connection if necessary.
 *
 * @param index The index of the poll file descriptor.
 */
void ServerEngine::pollFdError_(size_t &index)
{
	std::string error("");
	if (pollFds_[index].revents & POLLERR)
		error += "|POLLERR|";
	if (pollFds_[index].revents & POLLHUP)
		error += "|POLLHUP|";
	if (pollFds_[index].revents & POLLNVAL)
		error += "|POLLNVAL|";

	int			err = errno;
	std::string errMsg = strerror(err);
	Logger::log(Logger::DEBUG)
		<< "Client disconnected improperly: " << error << " on pollFds_["
		<< index << "]" << ", Fd[" << pollFds_[index].fd << "] , errno: " << err
		<< "," << errMsg << std::endl;

	if (!this->isPollFdServer_(this->pollFds_[index].fd)
		&& error.find("POLLNVAL") != std::string::npos)
	{
		this->pollFds_.erase(this->pollFds_.begin() + index);
		clients_.erase(clients_.begin() + clientIndex_);
		return;
	}
	else if (!this->isPollFdServer_(this->pollFds_[index].fd))
	{
		Logger::log(Logger::DEBUG)
			<< "Closing and deleting client socket: pollFds_[" << index
			<< "]: " << pollFds_[index].fd << std::endl;
		closeConnection_(index);
	}
	else
		restartServer_(index);
}

/**
 * @brief Initializes poll events.
 *
 * Calls the poll function to wait for events on the file descriptors.
 */
void ServerEngine::initializePollEvents()
{
	int pollCount = poll(pollFds_.data(), pollFds_.size(), POLL_TIMEOUT);
	if (pollCount == -1)
	{
		if (g_shutdown)
		{
			Logger::log(Logger::DEBUG)
				<< "Shutdown signal received, exiting poll." << std::endl;
			return;
		}
		Logger::log(Logger::ERROR, true)
			<< "poll() failed: (" << ft::toString(errno) << ") "
			<< strerror(errno) << std::endl;
		return;
	}
	Logger::log(Logger::DEBUG)
		<< "poll() returned " << pollCount << " events" << std::endl;
}

/**
 * @brief Reads a client request.
 *
 * Reads data from the client buffer and processes it.
 *
 * @param index The index of the client in the pollFds_ vector.
 */
void ServerEngine::readClientRequest_(size_t &index)
{
	Logger::log(Logger::INFO)
		<< "Reading client request at pollFds_[" << index << ']' << std::endl;

	try
	{
		if (clients_[clientIndex_].hasRequestReady() == false)
		{
			if (clients_[clientIndex_].isClosed() == true)
			{
				Logger::log(Logger::DEBUG)
					<< "Client disconnected: Erase clients_[" << clientIndex_
					<< "], " << "close and erase pollFds_[" << index << "]"
					<< std::endl;
			}
			return;
		}
	}
	catch (std::exception &e)
	{
		Logger::log(Logger::DEBUG, true)
			<< "Client.hasRequestReady resulted in error: " << e.what()
			<< std::endl;
		std::string response = HttpErrorHandler::getErrorPage(400, true);
		sendResponse_(index, response);
	}

	pollFds_[index].events = POLLOUT;
	Logger::log(Logger::DEBUG)
		<< "Read complete client request at pollFds_[" << index
		<< "] and set it to POLLOUT" << std::endl;
}

/**
 * @brief Sends a response to the client.
 *
 * Processes the client request and sends the appropriate response.
 *
 * @param index The index of the client in the pollFds_ vector.
 */
void ServerEngine::processClientRequest_(size_t &index)
{
	HttpRequest *request = NULL;
	std::string	 response;
	if (clients_[clientIndex_].isError() == true
		|| clients_[clientIndex_].isClosed() == true)
	{
		Logger::log(Logger::DEBUG, true)
			<< "processClientRequest_ got to request with error or closed. "
			<< std::endl;
		response = HttpErrorHandler::getErrorPage(400, true);
		sendResponse_(index, response);
		return;
	}

	try
	{
		std::string request_str = clients_[clientIndex_].extractRequestStr();
		request = new HttpRequest(RequestParser::parseRequest(request_str));
	}
	catch (std::exception &e)
	{
		response = HttpErrorHandler::getErrorPage(400, true);
		Logger::log(Logger::ERROR, true)
			<< "Failed to parse the request: " << e.what() << std::endl;
		sendResponse_(index, response);
		return;
	}

	if (request != NULL)
	{
		response = createResponse(*request);
		sendResponse_(index, response);
		delete request;
	}
}

/**
 * @brief Sends the response to the client.
 *
 * Sends the prepared response to the client.
 *
 * @param index The index of the client in the pollFds_ vector.
 * @param response The response string to be sent to the client.
 */
void ServerEngine::sendResponse_(size_t &index, const std::string &response)
{
	Logger::log(Logger::DEBUG) << "Sending response" << std::endl;
	int retCode
		= send(pollFds_[index].fd, response.c_str(), response.size(), 0);

	if (retCode < 0)
	{
		Logger::log(Logger::ERROR, true)
			<< "Failed to send response to client: (" << ft::toString(errno)
			<< ") " << strerror(errno) << std::endl;
		Logger::log(Logger::DEBUG)
			<< "Erase clients_[" << clientIndex_ << "], "
			<< "close and erase pollFds_[" << index << "]" << std::endl;
		closeConnection_(index);
	}
	else if (retCode == 0)
	{
		Logger::log(Logger::DEBUG)
			<< "Client disconnected: clients_[" << clientIndex_
			<< "] disconnected" << std::endl;
		Logger::log(Logger::DEBUG)
			<< "Erase clients_[" << clientIndex_ << "], "
			<< "close and erase pollFds_[" << index << "]" << std::endl;
		closeConnection_(index);
	}
	else
	{
		if (clients_[clientIndex_].isClosed())
		{
			closeConnection_(index);
		}
		else
		{
			pollFds_[index].events = POLLIN;
		}
	}
}

/**
 * @brief Processes poll events.
 *
 * Iterates through the pollFds_ vector and handles events for each file
 * descriptor.
 */
void ServerEngine::processPollEvents()
{
	for (pollIndex_ = 0; pollIndex_ < pollFds_.size(); pollIndex_++)
	{
		std::cout << "pollFds_ size is:" << pollFds_.size() << std::endl;
		clientIndex_ = pollIndex_ - totalServerInstances_;
		Logger::log(Logger::DEBUG)
			<< "clientIndex is set to " << clientIndex_ << std::endl;

		if (pollFds_[pollIndex_].revents & (POLLERR | POLLHUP | POLLNVAL))
		{
			pollFdError_(pollIndex_);
		}
		else if (pollFds_[pollIndex_].revents & POLLIN)
		{
			Logger::log(Logger::DEBUG) << "pollFds_[" << pollIndex_
									   << "] is ready for read" << std::endl;
			if (this->isPollFdServer_(pollFds_[pollIndex_].fd))
				acceptConnection_(pollIndex_);
			else
			{
				readClientRequest_(pollIndex_);
			}
		}
		else if (pollFds_[pollIndex_].revents & POLLOUT)
			processClientRequest_(pollIndex_);

		// TODO: rename indexes variables for consistency
		if (clientIndex_ > 0)
		{
			if (clients_[clientIndex_].isClosed() == true)
			{
				closeConnection_(pollIndex_);
			}
		}
	}
}

/**
 * @brief Starts the server engine.
 *
 * Initializes the server poll file descriptors and enters the main event loop.
 */
void ServerEngine::start()
{
	Logger::log(Logger::INFO) << "Starting the Server Engine" << std::endl;
	this->initServerPollFds_();

	while (!g_shutdown)
	{
		initializePollEvents();
		processPollEvents();
	}
}

/**
 * @brief Creates an HTTP response based on the request.
 *
 * @param request The HTTP request object.
 * @return The HTTP response as a string.
 */
std::string ServerEngine::createResponse(const HttpRequest &request)
{
	int serverIndex = this->findServer_(request.getHost(), request.getPort());

	if (serverIndex == -1)
		return HttpErrorHandler::getErrorPage(404, true);

	std::string method = request.getMethod();
	if (method == "GET" || method == "POST" || method == "DELETE")
		return HttpMethodHandler::handleRequest(
			request, servers_[serverIndex], method
		);
	else
		return HttpErrorHandler::getErrorPage(501, true);
}

/**
 * @brief Finds the server index based on host and port.
 *
 * @param host The host name.
 * @param port The port number.
 * @return The index of the server, or -1 if not found.
 */
int ServerEngine::findServer_(
	std::string const	 &host,
	unsigned short const &port
)
{
	for (size_t i = 0; i < this->totalServerInstances_; ++i)
	{
		std::vector<std::string> const &serverNames
			= this->servers_[i].getServerName();
		for (size_t j = 0; j < serverNames.size(); ++j)
		{
			if (serverNames[j] == host && this->servers_[i].getPort() == port)
				return i;
		}
	}
	return -1;
}

/**
 * @brief Closes a client connection.
 *
 * Removes the client from the clients_ vector and closes the file descriptor.
 *
 * @param index The index of the client in the pollFds_ vector.
 */
void ServerEngine::closeConnection_(size_t &index)
{
	Logger::log(Logger::INFO)
		<< "closeConnection_ gonna erase index: " << index << std::endl;
	this->clients_.erase(this->clients_.begin() + this->clientIndex_);
	close(this->pollFds_[index].fd);
	this->pollFds_.erase(this->pollFds_.begin() + index);
}
