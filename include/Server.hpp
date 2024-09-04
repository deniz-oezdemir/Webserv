#pragma once

#include <map>
#include <netinet/in.h>
#include <string>

#include "ConfigValue.hpp"

class Server
{
  public:
	Server(
		std::map<std::string, ConfigValue> const &server,
		unsigned int							  index = 0
	);
	Server(const Server &src);
	~Server(void);

	Server &operator=(const Server &rhs);

	void	initServer(void);

	// Getters
	unsigned int							 getPort(void) const;
	std::string								 getIpV4(void) const;
	unsigned long							 getClientMaxBodySize(void) const;
	std::string								 getRoot(void) const;
	std::vector<std::string>				 getIndex(void) const;
	std::vector<std::string>				 getServerName(void) const;
	std::map<std::string, ConfigValue> const getServerConfig(void) const;

	unsigned int	   getServerIndex(void) const;
	int const		  &getServerFd(void) const;
	sockaddr_in const &getServerAddr(void) const;

	// Get the location for an specific Error code
	bool getErrorPageValue(int &errorCode, std::string &location) const;
	bool getErrorPageValue(std::string &errorCode, std::string &location) const;

	// Check if the location is in the server config
	bool isThisLocation(const std::string &location) const;
	// Get the value of a key from a specific location map on the server config
	bool getThisLocationValue(
		const std::string		 &location,
		const std::string		 &key,
		std::vector<std::string> &value
	) const;

  private:
	Server(void);
	// Config Values
	uint16_t							port_;
	std::string						   &ipV4_;
	unsigned long						clientMaxBodySize_;
	std::string						   &root_;
	std::vector<std::string>		   &index_;
	std::vector<std::string>		   &serverName_;
	std::map<std::string, ConfigValue> &serverConfig_;

	unsigned int	 serverIndex_;
	int				 serverFd_;
	sockaddr_in		 serverAddr_;
	static int const BACKLOG_ = 10;

	void createSocket_();
	void bindSocket_();
	void listenSocket_();
};
