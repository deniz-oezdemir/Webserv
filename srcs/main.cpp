#include "ServerConfig.hpp"
#include "ServerInput.hpp"
#include "colors.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
	try
	{
		ServerInput input(argc, argv);
		if (input.hasThisFlag(ServerInput::HELP))
			std::cout << input.getHelpMessage() << std::endl;
		if (input.hasThisFlag(ServerInput::V_LITE) ||
			input.hasThisFlag(ServerInput::V_FULL))
			std::cout << input.getVersionMessage() << std::endl;
		ServerConfig config(input.getFilePath());
		config.parseFile(
			input.hasThisFlag(ServerInput::TEST),
			input.hasThisFlag(ServerInput::TEST_PRINT)
		);
	}
	catch (std::exception &e)
	{
		std::cerr << RED BOLD "Error:\t" RESET RED << e.what() << RESET
				  << std::endl;
	}
	return 0;
}
