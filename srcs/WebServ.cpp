#include "WebServ.hpp"

WebServ::WebServ() : numServers_(0) {}

WebServ::WebServ(std::vector<std::map<std::string, ConfigValue> > const &servers)
	: numServers_(servers.size())
{
	this->servers_.reserve(this->numServers_);
	std::vector<std::map<std::string, ConfigValue> >::const_iterator it;
	for (; it != servers.end(); it++)
		this->servers_.push_back(Server(*it));
}

WebServ::WebServ(WebServ const &src)
{
	*this = src;
}

WebServ	&WebServ::operator=(WebServ const &src)
{
	if (this != &src)
	{
		this->numServers_ = src.numServers_;
		this->pollFds_ = src.pollFds_;
		this->servers_ = src.servers_;
	}
	return *this;
}

WebServ::~WebServ() {}

void WebServ::handleClient_(int clientFd)
{
	char buffer[1000];
	long bytesRead = read(clientFd, buffer, sizeof(buffer));
	if (bytesRead < 0)
	{
		if (errno != EWOULDBLOCK && errno != EAGAIN)
		{
			throw std::runtime_error("Failed to read from client");
		}
		return;
	}
	else if (bytesRead == 0)
	{
		// Client disconnected
		close(clientFd);
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
	send(clientFd, response.c_str(), response.size(), 0);
}

void WebServ::acceptConnection_()
{
	int addrLen = sizeof(serverAddr_);
	int clientFd = accept(
		serverFd_, (struct sockaddr *)&serverAddr_, (socklen_t *)&addrLen
	);
	if (clientFd < 0)
	{
		if (errno != EWOULDBLOCK && errno != EAGAIN)
		{
			throw std::runtime_error("Failed to accept connection");
		}
		return;
	}

	// Set the client socket to non-blocking mode
	int flags = fcntl(clientFd, F_GETFL, 0);
	if (flags == -1 || fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		throw std::runtime_error("Failed to set non-blocking mode for client");
	}

	pollfd clientPollFd = {clientFd, POLLIN, 0};
	pollFds_.push_back(clientPollFd);
}

void WebServ::start()
{
	// Initialize pollFds_ vector
	pollFds_.clear();

	// Create pollfd struct for the server socket and add it to the vector
	pollfd serverPollFd = {serverFd_, POLLIN, 0};
	pollFds_.push_back(serverPollFd);

	while (true)
	{
		int pollCount = poll(pollFds_.data(), pollFds_.size(), -1);
		if (pollCount == -1)
		{
			throw std::runtime_error("poll() failed");
		}

		for (size_t i = 0; i < pollFds_.size(); ++i)
		{
			// Check if fd is ready for reading
			if (pollFds_[i].revents & POLLIN)
			{
				// If the file descriptor is the server socket, accept a new
				// client connection
				if (pollFds_[i].fd == serverFd_)
				{
					acceptConnection_();
				}
				else
				{
					// If the file descriptor is a client socket, handle client
					// I/O
					handleClient_(pollFds_[i].fd);
				}
			}
		}
	}
}
