#include "Logger.hpp"
#include "ServerConfig.hpp"
#include "ServerEngine.hpp"
#include "ServerInput.hpp"
#include "colors.hpp"
#include "signals.hpp"

#include <iostream>

bool g_shutdown = false;

static bool isHelpOrVersionFlags(ServerInput &input)
{
	if (!input.hasThisFlag(ServerInput::HELP)
		&& !input.hasThisFlag(ServerInput::V_LITE)
		&& !input.hasThisFlag(ServerInput::V_FULL))
		return false;
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
	return true;
}

static bool isTestMode(ServerInput &input, ServerConfig &config)
{
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
		return true;
	}
	return false;
}

int main(int argc, char *argv[])
{
	try
	{
		// Handle the input flags and the config file path
		ServerInput input(argc, argv);
		if (isHelpOrVersionFlags(input))
			return 0;
		// Config file parser. With test flags on the parseFile method will work
		// in test mode.
		ServerConfig config(input.getFilePath());
		config.parseFile(
			input.hasThisFlag(ServerInput::TEST),
			input.hasThisFlag(ServerInput::TEST_PRINT)
		);
		if (isTestMode(input, config))
			return 0;
		// Set the log level
		Logger::setLevel(config.getGeneralConfigValue("error_log"));
		// Set the signals handler
		signals::handleSignals();
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
