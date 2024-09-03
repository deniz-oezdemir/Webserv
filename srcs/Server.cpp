#include "../include/Server.hpp"

Server::Server(int port) : port_(port)
{
	createSocket();
	bindSocket();
	listenSocket();
}

Server::Server(const Server &src)
{
	*this = src;
}

Server::~Server(void)
{
	close(serverFd_);
}

Server &Server::operator=(const Server &rhs)
{
	if (this != &rhs)
	{
		serverFd_ = rhs.serverFd_;
		serverAddr_ = rhs.serverAddr_;
		port_ = rhs.port_;
	}
	return *this;
}

void Server::createSocket()
{
	serverFd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (serverFd_ == -1)
	{
		throw std::runtime_error("Failed to create socket");
	}

	// Set the socket to non-blocking mode
	int flags = fcntl(serverFd_, F_GETFL, 0);
	if (flags == -1 || fcntl(serverFd_, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		throw std::runtime_error("Failed to set non-blocking mode");
	}

	// Set the SO_REUSEADDR option
	int optval = 1;
	if (setsockopt(
			serverFd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)
		) == -1)
	{
		throw std::runtime_error("Failed to set SO_REUSEADDR");
	}
}

void Server::bindSocket()
{
	serverAddr_.sin_family = AF_INET;
	serverAddr_.sin_addr.s_addr = INADDR_ANY;
	serverAddr_.sin_port = htons(port_);

	// bind socket to server address
	if (bind(serverFd_, (struct sockaddr *)&serverAddr_, sizeof(serverAddr_)) <
		0)
	{
		throw std::runtime_error("Failed to bind socket");
	}
}

void Server::listenSocket()
{
	// listen on the server socket for incoming connections, maximum length of
	// queue of pending connections 10
	if (listen(serverFd_, 10) < 0)
	{
		throw std::runtime_error("Failed to listen on socket");
	}
}
