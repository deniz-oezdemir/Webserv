/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sebasnadu <johnavar@student.42berlin.de>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/26 17:48:50 by sebasnadu         #+#    #+#             */
/*   Updated: 2024/08/29 16:18:46 by sebasnadu        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

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
	void		   parseFile(bool isTest = false, bool isTestPrint = false);

	std::string filepath;

  private:
	ServerConfig();

	std::ifstream					   _file;
	std::map<std::string, std::string> _generalConfig;
	std::vector<std::map<std::string, ConfigValue> > _serversConfig;
};
