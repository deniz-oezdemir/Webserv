#include "ServerEngine.hpp"
#include "HttpErrorHandler.hpp"
#include "HttpMethodHandler.hpp"
#include "HttpRequest.hpp"
#include "Logger.hpp"
#include "macros.hpp"
#include "request_parser/RequestParser.hpp"
#include "utils.hpp"

#include <csignal>
#include <cstddef>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

extern bool g_shutdown;

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

	clients_.clear();
	clientIndex_ = 0;
}

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
			<< "| " << "Listen: " << this->servers_[globalServerIndex].getIPV4()
			<< ':' << this->servers_[globalServerIndex].getPort() << std::endl;
		++globalServerIndex;
	}
}

/* The serverFd is closed and initialized again, than the pollFds_ vector is
updated with the new file descriptor. */
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

void ServerEngine::initServerPollFds_(void)
{
	// Initialize pollFds_ vector
	pollFds_.clear();
	pollFds_.reserve(this->totalServerInstances_);

	Logger::log(Logger::DEBUG)
		<< "Initializing pollFds_ vector with ServerFds" << std::endl;

	// Create pollfd struct for the server socket and add it to the vector
	for (size_t i = 0; i < this->totalServerInstances_; ++i)
	{
		pollfd serverPollFd = {servers_[i].getServerFd(), POLLIN, 0};
		pollFds_.push_back(serverPollFd);
	}
}

bool ServerEngine::isPollFdServer_(int &fd)
{
	for (size_t i = 0; i < this->totalServerInstances_; ++i)
	{
		if (fd == this->servers_[i].getServerFd())
			return true;
	}
	return false;
}

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

	// Set the client socket to non-blocking mode
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

void ServerEngine::pollFdError_(size_t &index)
{
	std::string error("");
	if (pollFds_[index].revents & POLLERR)
		error += "|POLLERR|";
	if (pollFds_[index].revents & POLLHUP)
		error += "|POLLHUP|";
	if (pollFds_[index].revents & POLLNVAL)
		error += "|POLLNVAL|";

	// Retrieve and log the error using errno
	int			err = errno;
	std::string errMsg = strerror(err);
	Logger::log(Logger::DEBUG)
		<< "Client disconnected improperly: " << error << " on pollFds_["
		<< index << "]" << ", Fd[" << pollFds_[index].fd << "] , errno: " << err
		<< "," << errMsg << std::endl;

	if (!this->isPollFdServer_(this->pollFds_[index].fd)
		&& error.find("POLLNVAL") != std::string::npos)
	{
		// POLLNVAL: Fd was closed or never open
		this->pollFds_.erase(this->pollFds_.begin() + index);
		clients_.erase(clients_.begin() + clientIndex_);
		return;
	}
	else if (!this->isPollFdServer_(this->pollFds_[index].fd))
	{
		// POLLHUP: Hang up; client disconnected, nor more read or write on
		// socket possible POLLERR: Error; Error on fd
		Logger::log(Logger::DEBUG)
			<< "Closing and deleting client socket: pollFds_[" << index
			<< "]: " << pollFds_[index].fd << std::endl;
		closeConnection_(index);
	}
	else
		restartServer_(index);
}

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

void ServerEngine::readClientRequest_(size_t &index)
{
	Logger::log(Logger::INFO)
		<< "Reading client request at pollFds_[" << index << ']' << std::endl;

	if (clients_[clientIndex_].hasRequestReady() == false)
	{
		if (clients_[clientIndex_].isClosed() == true)
		{
			Logger::log(Logger::DEBUG)
				<< "Client disconnected:" << "Erase clients_[" << clientIndex_
				<< "], " << "close and erase pollFds_[" << index << "]"
				<< std::endl;
			closeConnection_(index);
		}
		return;
	}

	// After reading the request, prepare to send a response
	pollFds_[index].events = POLLOUT;
	Logger::log(Logger::DEBUG)
		<< "Read complete client request at pollFds_[" << index
		<< "] and set it to POLLOUT" << std::endl;
}

void ServerEngine::sendClientResponse_(size_t &index)
{
	// TODO: check if program  faster without  manual allocation of
	// HttpRequest
	HttpRequest *request = NULL;
	std::string	 response;
	try
	{
		request = new HttpRequest(RequestParser::parseRequest(
			clients_[clientIndex_].extractRequestStr()
		));
		Logger::log(Logger::DEBUG) << "Request received:\n"
								   << *request << std::flush;
	}
	catch (std::exception &e)
	{
		response = HttpErrorHandler::getErrorPage(400, true);
		Logger::log(Logger::ERROR, true)
			<< "Failed to parse the reguest: " << e.what() << std::endl;
	}
	if (request != NULL)
	{
		response = createResponse(*request);
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
			delete request;
			return;
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
			delete request;
			return;
		}
		if (clients_[clientIndex_].isClosed())
		{
			closeConnection_(index);
		}
		else
			pollFds_[index].events = POLLIN;
		// After sending the response, prepare to read the next request

		delete request;
	}
}

void ServerEngine::processPollEvents()
{
	for (size_t pollIndex = 0; pollIndex < pollFds_.size(); pollIndex++)
	{
		// Calculate offset only once per iteration
		// if (pollIndex >= totalServerInstances_)
		clientIndex_ = pollIndex - totalServerInstances_;
		Logger::log(Logger::DEBUG)
			<< "clientIndex is set to " << clientIndex_ << std::endl;

		// Check if fd has some errors(pollerr, pollnval) or if the
		// connection was broken(pollhup)
		if (pollFds_[pollIndex].revents & (POLLERR | POLLHUP | POLLNVAL))
		{
			pollFdError_(pollIndex);
		}
		else if (pollFds_[pollIndex].revents & POLLIN)
		{
			Logger::log(Logger::DEBUG) << "pollFds_[" << pollIndex
									   << "] is ready for read" << std::endl;
			// If fd is a server socket accept a new client connection
			if (this->isPollFdServer_(pollFds_[pollIndex].fd))
				acceptConnection_(pollIndex);
			else
				readClientRequest_(pollIndex);
		}
		else if (pollFds_[pollIndex].revents & POLLOUT)
			sendClientResponse_(pollIndex);
	}
}

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

void ServerEngine::closeConnection_(size_t &index)
{
	this->clients_.erase(this->clients_.begin() + this->clientIndex_);
	close(this->pollFds_[index].fd);
	this->pollFds_.erase(this->pollFds_.begin() + index);
}
