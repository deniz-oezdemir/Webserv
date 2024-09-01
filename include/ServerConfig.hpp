#pragma once

#include <array>
#include <fstream>
#include <map>
#include <stack>
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
	bool		   isConfigOK(void) const;
	void		   parseFile(bool isTest = false, bool isTestPrint = false);
	void		   printConfig(void);
	std::string	   getGeneralConfigValue(std::string const &key) const;
	bool		   getAllServersConfig(
				  std::vector<std::map<std::string, ConfigValue> > &serversConfig
			  ) const;
	bool getServerConfigValue(
		unsigned int	   serverIndex,
		std::string const &key,
		ConfigValue		  &value
	) const;

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
	void _parseLocationBlock(
		std::vector<std::string> &tokens,
		std::string				 &line,
		unsigned int			 &lineIndex,
		std::stack<bool>		 &brackets,
		bool					 &isTest,
		bool					 &isTestPrint
	);
	void _checkGeneralConfig(bool isTest, bool isTestPrint);
	void _checkServersConfig(bool isTest, bool isTestPrint);

	std::ifstream									_file;
	std::map<std::string, std::string>				_generalConfig;
	std::vector<std::map<std::string, ConfigValue> > _serversConfig;
	bool											_isConfigOK;
	void											_setListenDirective(
												   std::vector<std::string> const &tokens,
												   unsigned int							  &lineIndex,
												   bool							  &isTest,
												   bool							  &isTestPrint
											   );
};
