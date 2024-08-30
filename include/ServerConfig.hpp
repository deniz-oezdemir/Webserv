#pragma once

#include <array>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "ConfigValue.hpp"

class ServerConfig
{
  public:
	ServerConfig(std::string const &filepath);
	ServerConfig(ServerConfig const &src);
	ServerConfig &operator=(ServerConfig const &src);
	~ServerConfig();

	std::ifstream &getFile(void);
	bool		   getIsConfigOK(void) const;
	void		   parseFile(bool isTest = false, bool isTestPrint = false);
	void		   printConfig(void);

	std::string								filepath;
	static std::array<std::string, 4> const validLogLevels;

  private:
	ServerConfig();

	void _initGeneralConfig(void);
	void _initServersConfig(void);
	bool _checkValues(
		std::vector<std::string> const &line,
		unsigned int					maxSize,
		unsigned int					lineIndex,
		bool							isTest,
		bool							isTestPrint
	);
	void _errorHandler(
		std::string const &message,
		unsigned int	   lineIndex,
		bool			   isTest,
		bool			   isTestPrint
	);

	std::ifstream									_file;
	std::map<std::string, std::string>				_generalConfig;
	std::vector<std::map<std::string, ConfigValue> > _serversConfig;
	bool											_isConfigOK;
};
