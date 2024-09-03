#include "Server.hpp"
#include "utils.hpp"

#include <stdexcept>

Server::Server(std::map<std::string, ConfigValue> &server, bool isDefault)
	: port_(ft::strToUint16(server["listen"].getMapValue("port")[0])),
	  ipV6_(server["listen"].getMapValue("host")[0]),
	  clientMaxBodySize_(
		  ft::stringToULong(server["client_max_body_size"].getVectorValue(0))
	  ),
	  root_(server["root"].getVectorValue(0)),
	  index_(server["index"].getVector()),
	  serverName_(server["server_name"].getVector()), serverConfig_(server),
	  isDefault_(isDefault)
{
	createSocket();
	bindSocket();
	listenSocket();
}

Server::Server(const Server &src)
	: port_(src.port_), ipV6_(src.ipV6_),
	  clientMaxBodySize_(src.clientMaxBodySize_), root_(src.root_),
	  index_(src.index_), serverName_(src.serverName_),
	  serverConfig_(src.serverConfig_),
	  isDefault_(src.isDefault_), serverFd_(src.serverFd_),
	  serverAddr_(src.serverAddr_)
{
	createSocket();
	bindSocket();
	listenSocket();
}

Server::~Server(void)
{
	close(serverFd_);
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
	if (setsockopt(serverFd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))
		== -1)
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
	if (bind(serverFd_, (struct sockaddr *)&serverAddr_, sizeof(serverAddr_))
		< 0)
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
