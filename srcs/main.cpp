#include "Logger.hpp"
#include "ServerConfig.hpp"
#include "ServerEngine.hpp"
#include "ServerInput.hpp"
#include "colors.hpp"

#include <iostream>

int main(int argc, char *argv[])
{
	try
	{
		// Handle the input flags and the config file
		ServerInput input(argc, argv);
		// If the user asked for help, print the help message
		if (input.hasThisFlag(ServerInput::HELP))
		{
			std::cout << input.getHelpMessage() << std::endl;
		}
		// If the user asked for the version, print the version message
		if (input.hasThisFlag(ServerInput::V_LITE)
			|| input.hasThisFlag(ServerInput::V_FULL))
		{
			std::cout << input.getVersionMessage() << std::endl;
		}
		// Open and parse the config file, if the user ask for a test of the
		// config file, the parseFile method will work in test mode.
		ServerConfig config(input.getFilePath());
		config.parseFile(
			input.hasThisFlag(ServerInput::TEST),
			input.hasThisFlag(ServerInput::TEST_PRINT)
		);
		// If the user asked for a test of the config file, print the config and
		// exit
		if (input.hasThisFlag(ServerInput::TEST)
			|| input.hasThisFlag(ServerInput::TEST_PRINT))
		{
			if (config.isConfigOK())
				std::cout << PURPLE "<WebServ> " << RESET "Test finished: "
						  << GREEN BOLD "Config OK!" RESET << std::endl;
			if (input.hasThisFlag(ServerInput::TEST_PRINT))
				config.printConfig();
			return 0;
		}
		// Set the log level
		Logger::setLevel(config.getGeneralConfigValue("error_log"));

		// Init and start the server(s)
		ServerEngine engine(config.getAllServersConfig());
		engine.start();
	}
	catch (std::exception &e)
	{
		std::cerr << RED BOLD "Error:\t" RESET RED << e.what() << RESET
				  << std::endl;
	}
	return 0;
}
