/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerInput.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sebasnadu <johnavar@student.42berlin.de>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/27 14:35:22 by sebasnadu         #+#    #+#             */
/*   Updated: 2024/08/27 15:11:16 by sebasnadu        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <string>

class ServerInput
{
  public:
	ServerInput();
	ServerInput(int argc, char** argv);
	ServerInput(ServerInput const& src);
	~ServerInput();

	ServerInput& operator=(ServerInput const& src);

	typedef enum e_serverFlags
	{
		NONE = 0x00,
		VERSION = 0x01,
		HELP = 0x02,
		TEST = 0x04,
		TEST_PRINT = 0x08,
	} t_serverFlags;

  private:
	t_serverFlags _flags;
	std::string	  _filepath;
};
