#pragma once

#include <string>

class ServerConfig
{
  public:
	ServerConfig(std::string const& filepath);
	ServerConfig(ServerConfig const& src);
	ServerConfig& operator=(ServerConfig const& src);

  private:
	ServerConfig();
};
