/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sebasnadu <johnavar@student.42berlin.de>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/26 17:48:50 by sebasnadu         #+#    #+#             */
/*   Updated: 2024/08/26 21:26:16 by sebasnadu        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
