/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sebasnadu <johnavar@student.42berlin.de>   +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/28 09:53:44 by sebasnadu         #+#    #+#             */
/*   Updated: 2024/08/29 13:24:48 by sebasnadu        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "colors.hpp"
#include "ServerInput.hpp"
#include "ServerConfig.hpp"

#include <iostream>

int main(int argc, char* argv[])
{
	try
	{
		ServerInput	input(argc, argv);
		if (input.hasThisFlag(ServerInput::HELP))
			std::cout << input.getHelpMessage() << std::endl;
		if (input.hasThisFlag(ServerInput::V_LITE) ||
			input.hasThisFlag(ServerInput::V_FULL))
			std::cout << input.getVersionMessage() << std::endl;
		if (input.hasThisFlag(ServerInput::TEST))
			std::cout << "test" << std::endl;
		if (input.hasThisFlag(ServerInput::TEST_PRINT))
			std::cout << "test print" << std::endl;
		ServerConfig	config(input.getFilePath());
	}
	catch (std::exception& e)
	{
		std::cerr << RED BOLD "Error:\t" RESET RED<< e.what() << RESET
				  << std::endl;
	}
	return 0;
}
