#include "Server.hpp"
#include "Logger.hpp"
#include "ServerException.hpp"
#include "utils.hpp"

#include <fcntl.h>
#include <netdb.h>
#include <unistd.h>

Server::Server(
	std::map<std::string, ConfigValue> const &server,
	unsigned int							  index,
	unsigned int							  listenIndex
)
	: port_(ft::strToUShort(server.at("listen").getMapValue("port")[listenIndex]
	  )),
	  ipV4_(const_cast<std::string &>(server.at("listen").getMapValue("host"
	  )[listenIndex])),
	  clientMaxBodySize_(
		  ft::stringToULong(server.at("client_max_body_size").getVectorValue(0))
	  ),
	  root_(const_cast<std::string &>(server.at("root").getVectorValue(0))),
	  index_(
		  const_cast<std::vector<std::string> &>(server.at("index").getVector())
	  ),
	  serverName_(const_cast<std::vector<std::string> &>(
		  server.at("server_name").getVector()
	  )),
	  serverConfig_(const_cast<std::map<std::string, ConfigValue> &>(server)),
	  serverIndex_(index)
{
}

Server::Server(const Server &src)
	: port_(src.port_), ipV4_(src.ipV4_),
	  clientMaxBodySize_(src.clientMaxBodySize_), root_(src.root_),
	  index_(src.index_), serverName_(src.serverName_),
	  serverConfig_(src.serverConfig_), serverIndex_(src.serverIndex_),
	  serverFd_(src.serverFd_), serverAddr_(src.serverAddr_)
{
}

Server &Server::operator=(const Server &src)
{
	if (this != &src)
	{
		port_ = src.port_;
		ipV4_ = src.ipV4_;
		clientMaxBodySize_ = src.clientMaxBodySize_;
		root_ = src.root_;
		index_ = src.index_;
		serverName_ = src.serverName_;
		serverConfig_ = src.serverConfig_;
		serverIndex_ = src.serverIndex_;
		serverFd_ = src.serverFd_;
		serverAddr_ = src.serverAddr_;
	}
	return *this;
}

Server::~Server(void)
{
	// Close the server socket if it is open and set the file descriptor to -1
	// to indicate that the socket is closed
	if (serverFd_ >= 0)
	{
		this->closeServer();
	}
}

void Server::init(void)
{
	createSocket_();
	bindSocket_();
	listenSocket_();
}

void Server::closeServer(void)
{
	Logger::log(Logger::DEBUG) << "Closing server socket" << std::endl;
	if (serverFd_ >= 0)
	{
		close(serverFd_);
		serverFd_ = -1;
	}
}

void Server::resetServer(void)
{
	closeServer();
	Logger::log(Logger::DEBUG) << "initializing server socket" << std::endl;
	init();
}

void Server::createSocket_()
{
	this->serverFd_ = socket(AF_INET, SOCK_STREAM, 0);
	if (this->serverFd_ == -1)
		throw ServerException(
			"Failed to create socket on the Server[%]",
			errno,
			ft::toString(serverIndex_)
		);

	try
	{
		// Set the socket to non-blocking mode
		int flags = fcntl(serverFd_, F_GETFL, 0);
		if (flags == -1)
			throw ServerException(
				"Failed to get socket flags on the Server[%]",
				errno,
				ft::toString(serverIndex_)
			);
		if (fcntl(serverFd_, F_SETFL, flags | O_NONBLOCK) == -1)
			throw ServerException(
				"Failed to set socket flags on the Server[%]",
				errno,
				ft::toString(serverIndex_)
			);

		// Set the SO_REUSEADDR option, allowing the server to bind to an
		// address that is already in use.
		int optval = 1;
		if (setsockopt(
				serverFd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)
			)
			== -1)
			throw ServerException(
				"Failed to set socket option REUSEADDR on the Server[%]",
				errno,
				ft::toString(serverIndex_)
			);
		// Set the SO_KEEPALIVE option, enabling the server to send keepalive
		// messages on the connection to detect a dead peer and close the
		// connection.
		if (setsockopt(
				serverFd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval)
			)
			== -1)
			throw ServerException(
				"Failed to set socket option KEEPALIVE on the Server[%]",
				errno,
				ft::toString(serverIndex_)
			);
	}
	catch (std::exception &e)
	{
		if (serverFd_ != -1)
			close(serverFd_);
		throw;
	}
}

// If the server is the first server block in the configuration file
// (default server), bind the socket to INADDR_ANY to listen on all IPv4.
// Otherwise, bind the socket to the specified IPv4 address.
void Server::bindSocket_()
{
	if (serverIndex_ == 0)
	{
		// Set the address family to IPv4
		serverAddr_.sin_family = AF_INET;
		// Set the server address to listen on all IPv4 addresses
		serverAddr_.sin_addr.s_addr = INADDR_ANY;
		// Set the server address to listen on the specified port
		serverAddr_.sin_port = htons(port_);
	}
	else
	{
		// hints is a struct that contains input parameters for getaddrinfo
		struct addrinfo hints;
		// result is a struct that contains output parameters for getaddrinfo
		struct addrinfo *result;

		memset(&hints, 0, sizeof(hints));
		// Set the address family to IPv4
		hints.ai_family = AF_INET;
		// Set the socket type to stream socket (TCP)
		hints.ai_socktype = SOCK_STREAM;
		// Use the IP address of the local host
		hints.ai_flags = AI_PASSIVE;

		// Convert the IPv4 address to a network address structure using
		// getaddrinfo and store the result in the result struct pointer
		int status = getaddrinfo(ipV4_.c_str(), NULL, &hints, &result);
		if (status != 0)
			throw ServerException(
				"Failed to get address info with the IPv4(%) on the Server["
					+ ft::toString(serverIndex_) + "]",
				status,
				ipV4_
			);

		// Copy the server address from the result struct to the server address
		serverAddr_ = *(struct sockaddr_in *)result->ai_addr;
		serverAddr_.sin_port = htons(port_);

		// Free the memory allocated for the result struct
		freeaddrinfo(result);
	}

	// Bind the socket to the server address
	if (bind(serverFd_, (struct sockaddr *)&serverAddr_, sizeof(serverAddr_))
		< 0)
	{
		throw ServerException(
			"Failed to bind socket on the Server[%]",
			errno,
			ft::toString(serverIndex_)
		);
	}
}

void Server::listenSocket_()
{
	// listen on the server socket for incoming connections, maximum length of
	// queue of pending connections 10
	if (listen(serverFd_, this->BACKLOG_) < 0)
	{
		throw ServerException(
			"Failed to listen the socket on the Server[%]",
			errno,
			ft::toString(serverIndex_)
		);
	}
}

// Getters

unsigned int Server::getPort(void) const
{
	return port_;
}

std::string Server::getIPV4(void) const
{
	return ipV4_;
}

unsigned long Server::getClientMaxBodySize(void) const
{
	return clientMaxBodySize_;
}

std::string Server::getRoot(void) const
{
	return root_;
}

std::vector<std::string> Server::getIndex(void) const
{
	return index_;
}

std::vector<std::string> Server::getServerName(void) const
{
	return serverName_;
}

std::map<std::string, ConfigValue> const Server::getServerConfig(void) const
{
	return serverConfig_;
}

unsigned int Server::getServerIndex(void) const
{
	return serverIndex_;
}

int const &Server::getServerFd(void) const
{
	return serverFd_;
}

sockaddr_in const &Server::getServerAddr(void) const
{
	return serverAddr_;
}

bool Server::getErrorPageValue(int &errorCode, std::string &location) const
{
	std::map<std::string, ConfigValue>::const_iterator it;
	it = serverConfig_.find(ft::toString(errorCode));
	if (it == serverConfig_.end())
	{
		location = "";
		return false;
	}
	std::vector<std::string> errorPage = it->second.getVector();
	location = errorPage[0];
	return true;
}

bool Server::getErrorPageValue(std::string &errorCode, std::string &location)
	const
{
	std::map<std::string, ConfigValue>::const_iterator it;
	it = serverConfig_.find(errorCode);
	if (it == serverConfig_.end())
	{
		location = "";
		return false;
	}
	std::vector<std::string> errorPage = it->second.getVector();
	location = errorPage[0];
	return true;
}

bool Server::isThisLocation(const std::string &location) const
{
	std::map<std::string, ConfigValue>::const_iterator it;
	it = serverConfig_.find(location);
	if (it == serverConfig_.end())
		return false;
	return true;
}

bool Server::getThisLocationValue(
	std::string const		 &location,
	std::string const		 &key,
	std::vector<std::string> &value
) const
{
	std::map<std::string, ConfigValue>::const_iterator it;
	it = serverConfig_.find(location);
	if (it == serverConfig_.end())
	{
		value.clear();
		return false;
	}
	if (!it->second.getMapValue(key, value))
	{
		value.clear();
		return false;
	}
	return true;
}

// clang-format off
std::map<std::string, std::vector<std::string> > const
// clang-format on
Server::getThisLocation(std::string const &location) const
{
	std::map<std::string, ConfigValue>::const_iterator it;
	it = serverConfig_.find(location);
	if (it == serverConfig_.end())
	{
		// clang-format off
		return std::map<std::string, std::vector<std::string> >();
		// clang-format on
	}
	return it->second.getMap();
}

void Server::setPort(std::string const &port)
{
	this->port_ = ft::strToUShort(port);
}

void Server::setIPV4(std::string const &ipV4)
{
	this->ipV4_ = ipV4;
}

void Server::setRoot(const std::string &root)
{
	root_ = root;
}
