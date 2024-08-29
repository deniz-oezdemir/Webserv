#pragma once

#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <array>

#include "ConfigValue.hpp"

class ServerConfig
{
  public:
	ServerConfig(std::string const &filepath);
	ServerConfig(ServerConfig const &src);
	ServerConfig &operator=(ServerConfig const &src);
	~ServerConfig();

	std::ifstream &getFile(void);
	void		   parseFile(bool isTest = false, bool isTestPrint = false);

	std::string filepath;
	static std::array<std::string, 4> const	validLogLevels;

  private:
	ServerConfig();

	void	_initGeneralConfig(void);
	void	_initServersConfig(void);

	std::ifstream					   													_file;
	std::map<std::string, std::string>								_generalConfig;
	std::vector<std::map<std::string, ConfigValue> >	_serversConfig;
};
