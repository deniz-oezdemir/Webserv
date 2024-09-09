#include "signals.hpp"
#include "Logger.hpp"

#include <csignal>
#include <cstdlib>
#include <iostream>

namespace signals
{

static void handleInterrupt(int signal)
{
	if (signal == SIGINT)
	{
		std::cout << '\n';
		Logger::log(Logger::DEBUG)
			<< "SIGINT received. Exiting..." << std::endl;
		std::exit(signal);
	}
}

static void handleQuit(int signal)
{
	if (signal == SIGQUIT)
	{
		Logger::log(Logger::DEBUG)
			<< "SIGQUIT received. Exiting..." << std::endl;
		std::exit(signal);
	}
}

static void handleTerminate(int signal)
{
	if (signal == SIGTERM)
	{
		Logger::log(Logger::DEBUG)
			<< "SIGTERM received. Exiting..." << std::endl;
		std::exit(signal);
	}
}

// TODO: Implement the SIGHUP signal to change the status from a global
// variable to restart the config and servers.
static void handleHangup(int signal)
{
	if (signal == SIGHUP)
	{
		Logger::log(Logger::DEBUG)
			<< "SIGHUP received. Reloading the configuration file and "
			   "restarting the server..."
			<< std::endl;
	}
}

static void handlePipe(int signal)
{
	if (signal == SIGPIPE)
	{
		Logger::log(Logger::DEBUG)
			<< "SIGPIPE received. Client disconnected unexpectedly."
			<< std::endl;
	}
}

void handleSignals(void)
{
	std::signal(SIGINT, handleInterrupt);
	std::signal(SIGQUIT, handleQuit);
	std::signal(SIGTERM, handleTerminate);
	std::signal(SIGHUP, handleHangup);
	std::signal(SIGPIPE, handlePipe);
}

} // namespace signals
