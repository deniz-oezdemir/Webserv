#include "WebServ.hpp"
#include "HttpRequest.hpp"
#include "Logger.hpp"
#include "RequestParser.hpp"
#include "utils.hpp"

#include <fcntl.h>
#include <unistd.h>

WebServ::WebServ() : numServers_(0) {}

WebServ::WebServ(std::vector<std::map<std::string, ConfigValue> > const &servers)
	: numServers_(servers.size())
{
	this->servers_.reserve(this->numServers_);
	for (size_t i = 0; i < this->numServers_; ++i)
	{
		this->servers_.push_back(Server(servers[i], i));
		this->servers_[i].initServer();
	}
}

WebServ::~WebServ() {}

void WebServ::initPollFds_(void)
{
	// Initialize pollFds_ vector
	pollFds_.clear();
	pollFds_.reserve(this->numServers_);

	// Create pollfd struct for the server socket and add it to the vector
	for (size_t i = 0; i < this->numServers_; ++i)
	{
		pollfd serverPollFd = {servers_[i].getServerFd(), POLLIN, 0};
		pollFds_.push_back(serverPollFd);
	}
}

bool WebServ::isPollFdServer_(int &fd)
{
	for (size_t i = 0; i < this->numServers_; ++i)
	{
		if (fd == this->servers_[i].getServerFd())
			return true;
	}
	return false;
}

void WebServ::acceptConnection_(size_t &index)
{
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

	pollfd clientPollFd = {clientFd, POLLIN, 0};
	pollFds_.push_back(clientPollFd);
}

void WebServ::handleClient_(size_t &index)
{
	static size_t const bufferSize = 4096;
	char				buffer[bufferSize];
	long bytesRead = read(this->pollFds_[index].fd, buffer, sizeof(buffer));
	if (bytesRead < 0)
	{
		if (errno != EWOULDBLOCK && errno != EAGAIN)
		{
			Logger::log(Logger::ERROR, true)
				<< "Failed to read from client: (" << ft::toString(errno)
				<< ") " << strerror(errno) << std::endl;
		}
		return;
	}
	else if (bytesRead == 0)
	{
		// Client disconnected
		Logger::log(Logger::DEBUG)
			<< "Client disconnected: pollFds_[" << index << "]" << std::endl;
		close(pollFds_[index].fd);
		pollFds_.erase(pollFds_.begin() + index);
		return;
	}

	// Parse the request
	try
	{
		std::string requestStr(buffer, bytesRead);
		HttpRequest request = RequestParser::parseRequest(requestStr);
		std::cout << "Hello from server. Your message was: " << buffer;

		std::cout << std::endl
				  << "Request received: " << std::endl
				  << request << std::endl;
	}
	catch (std::exception &e)
	{
		std::cerr << RED BOLD "Error:\t" RESET RED << e.what() << RESET
				  << std::endl;
	}
	std::string response = "Have a good day.\n";
	int			retCode
		= send(pollFds_[index].fd, response.c_str(), response.size(), 0);
	if (retCode < 0)
	{
		Logger::log(Logger::ERROR, true)
			<< "Failed to send response to client: (" << ft::toString(errno)
			<< ") " << strerror(errno) << std::endl;
		close(pollFds_[index].fd);
		pollFds_.erase(pollFds_.begin() + index);
		return;
	}
	else if (retCode == 0)
	{
		Logger::log(Logger::DEBUG)
			<< "Failed to send response to client: Connection closed by client"
			<< "pollFds_[" << index << "]" << std::endl;
		close(pollFds_[index].fd);
		pollFds_.erase(pollFds_.begin() + index);
		return;
	}
}

void WebServ::start()
{
	this->initPollFds_();
	while (true)
	{
		int pollCount = poll(pollFds_.data(), pollFds_.size(), -1);
		if (pollCount == -1)
		{
			Logger::log(Logger::ERROR, true)
				<< "poll() failed: (" << ft::toString(errno) << ") "
				<< strerror(errno) << std::endl;
			continue;
		}

		for (size_t i = 0; i < pollFds_.size(); ++i)
		{
			// Check if fd is ready for reading
			if (pollFds_[i].revents & POLLIN)
			{
				// If the file descriptor is the server socket, accept a new
				// client connection
				if (this->isPollFdServer_(pollFds_[i].fd))
				{
					acceptConnection_(i);
				}
				else
				{
					// If the file descriptor is a client socket, handle client
					// I/O
					handleClient_(i);
				}
			}
			if (pollFds_[i].revents & (POLLERR | POLLHUP | POLLNVAL))
			{
			 std::string error("");
				if (pollFds_[i].revents & POLLERR)
					error += "POLLERR ";
				if (pollFds_[i].revents & POLLHUP)
					error += "POLLHUP ";
				if (pollFds_[i].revents & POLLNVAL)
					error += "POLLNVAL";
				// Handle error
				Logger::log(Logger::ERROR, true)
					<< "Descriptor error on fd: " << pollFds_[i].fd << " : ("
					<< error << ") " << std::endl;
				if (!this->isPollFdServer_(this->pollFds_[i].fd))
				{
					close(pollFds_[i].fd);
					this->pollFds_.erase(this->pollFds_.begin() + i);
				}
				// TODO: restartDescriptor from Server class and then update
				// pollFds_ if it is an serveFd
			}
		}
	}
}
