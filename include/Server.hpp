#pragma once

#include <fcntl.h>
#include <map>
#include <netinet/in.h>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include "ConfigValue.hpp"

class Server
{
  public:
	Server(std::map<std::string, ConfigValue> &server, bool isDefault = false);
	Server(const Server &src);
	~Server(void);


  protected:
  private:
	Server(void);
	Server &operator=(const Server &rhs);

	// Config Values
	uint16_t const					port_;
	std::string const			   &ipV6_;
	unsigned long const				clientMaxBodySize_;
	std::string const			   &root_;
	std::vector<std::string> const &index_;
	std::vector<std::string> const &serverName_;
	std::map<std::string, ConfigValue> const &serverConfig_;

	bool						isDefault_;
	int								serverFd_;
	sockaddr_in						serverAddr_;

	void createSocket();
	void bindSocket();
	void listenSocket();
};
