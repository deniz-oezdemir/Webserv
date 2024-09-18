#pragma once

#include <array>
#include <fstream>
#include <map>
#include <stack>
#include <string>
#include <vector>

#include "ConfigValue.hpp"

// ServerConfig class is used to parse and check the configuration file and
// store the values in a map.
class ServerConfig
{
  public:
	ServerConfig(std::string const &filepath);
	ServerConfig(ServerConfig const &src);
	ServerConfig &operator=(ServerConfig const &src);
	~ServerConfig();

	std::ifstream &getFile(void);
	bool		   isConfigOK(void) const;

	// Parse the configuration file and store the values in the map. If isTest
	// and/or isTestPrint is true, it will print the error messages without
	// stopping the program.
	void parseFile(bool isTest = false, bool isTestPrint = false);
	void printConfig(void);

	// Get the value of a key in the general configuration map.
	std::string getGeneralConfigValue(std::string const &key) const;

	// Get all servers stored in a vector of maps.
	bool getAllServersConfig(
		std::vector<std::map<std::string, ConfigValue> > &serversConfig
	) const;

	std::vector<std::map<std::string, ConfigValue> > const &
	getAllServersConfig(void) const;

	// Get the value of a key in a server[serverIndex] configuration map.
	bool getServerConfigValue(
		unsigned int	   serverIndex,
		std::string const &key,
		ConfigValue		  &value
	) const;

	static std::array<std::string, 4> const validLogLevels;

  private:
	ServerConfig();

	void initGeneralConfig_(void);
	void initServersConfig_(void);
	void initLocationConfig_(
		std::map<std::string, std::vector<std::string> > &location
	);

	void errorHandler_(
		std::string const &message,
		unsigned int	   lineIndex,
		bool			   isTest,
		bool			   isTestPrint
	);

	bool checkValues_(
		std::vector<std::string> const &line,
		unsigned int					maxSize,
		unsigned int					lineIndex,
		bool							isTest,
		bool							isTestPrint
	);

	bool isValidErrorCode_(std::string const &code);
	bool isURI_(std::string const &uri);
	bool isURL_(std::string const &url);
	bool isExecutable_(std::string const &path);
	bool isDirectory_(std::string const &path);

	bool checkDirective_(std::vector<std::string> const &tokens);
	bool checkLimitExcept_(std::vector<std::string> const &tokens);
	bool checkAutoIndex_(std::vector<std::string> const &tokens);
	bool checkReturn_(std::vector<std::string> const &tokens);
	bool checkCgi_(std::vector<std::string> const &tokens);
	bool checkUploadStore_(std::vector<std::string> const &tokens);
	bool checkClientMaxBodySize_(std::vector<std::string> const &tokens);
	bool checkRoot_(std::vector<std::string> const &tokens);
	bool checkServerListenUnique_(
		std::string const &hoist,
		std::string const &port,
		unsigned int	  &lineIndex,
		bool			  &isTest,
		bool			  &isTestPrint
	);

	bool checkServerNameUnique_(std::string const &tokens);
	bool checkListenUnique_(
		std::vector<std::string> const &host,
		std::vector<std::string> const &port
	);

	void parseLocationBlock_(
		std::vector<std::string> &tokens,
		std::string				 &line,
		unsigned int			 &lineIndex,
		std::stack<bool>		 &brackets,
		bool					 &isTest,
		bool					 &isTestPrint
	);
	void checkGeneralConfig_(bool isTest, bool isTestPrint);
	void checkServersConfig_(bool isTest, bool isTestPrint);

	std::string										filepath_;
	std::ifstream									file_;
	std::map<std::string, std::string>				generalConfig_;
	std::vector<std::map<std::string, ConfigValue> > serversConfig_;
	bool											isConfigOK_;
	void											setListenDirective_(
												   std::vector<std::string> const &tokens,
												   unsigned int					  &lineIndex,
												   bool							  &isTest,
												   bool							  &isTestPrint
											   );
};
