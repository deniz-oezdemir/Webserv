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
	WebServ(std::vector<std::map<std::string, ConfigValue> > const &servers);
	~WebServ();

	void start(void);

  private:
	WebServ(WebServ const &src);
	WebServ &operator=(WebServ const &src);

	unsigned int		numServers_;
	std::vector<pollfd> pollFds_;
	std::vector<Server> servers_;

	void initPollFds_(void);
	bool isPollFdServer_(int &fd);
	void handleClient_(size_t &index);
	void acceptConnection_(size_t &index);
};
