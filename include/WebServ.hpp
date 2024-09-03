#pragma once

#include "ConfigValue.hpp"
#include "Server.hpp"

#include <map>
#include <poll.h>
#include <string>

class WebServ
{
  public:
	WebServ();
	WebServ(WebServ const &src);
	WebServ(std::vector<std::map<std::string, ConfigValue> > const &servers);
	~WebServ();

	WebServ &operator=(WebServ const &src);

	void start(void);

  private:
	unsigned int		numServers_;
	std::vector<pollfd> pollFds_;
	std::vector<Server> servers_;

	void handleClient_(int clientFd);
	void acceptConnection_(void);
};
