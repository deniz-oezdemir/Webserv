#include "Logger.hpp"
#include "colors.hpp"

#include <ctime>
#include <iostream>

Logger::Logger(void) : _level(INFO){};

Logger::~Logger(void){};

void Logger::setLevel(Level level)
{
	_level = level;
}

void Logger::setLevel(std::string const &level)
{
	if (level == "debug")
		_level = DEBUG;
	else if (level == "info")
		_level = INFO;
	else if (level == "warm")
		_level = WARN;
	else if (level == "error")
		_level = ERROR;
}

Logger::Level Logger::getLevel(void) const
{
	return _level;
}

Logger::Level Logger::getLevel(std::string const &level) const
{
	if (level == "debug")
		return this->DEBUG;
	else if (level == "info")
		return this->INFO;
	else if (level == "warn")
		return this->WARN;
	else if (level == "error")
		return this->ERROR;
	return this->INFO;
}

// This function logs the message to the console if the level is equal or higher
// than the set level. The message is colored depending on the color string
// passed. If isError is true, the message is logged to std::cerr, otherwise it
// is logged to std::cout.
void Logger::log(
	Level const		   level,
	std::string const &message,
	std::string const &color,
	bool const		   isError
)
{
	if (level < _level)
		return;
	time_t	   rawTime;
	struct tm *timeInfo;
	char	   buffTime[32];

	time(&rawTime);
	timeInfo = localtime(&rawTime);
	if (strftime(buffTime, sizeof(buffTime), "%T", timeInfo) == 0)
		buffTime[0] = '\0';
	if (isError)
		std::cerr << CYAN "[" << buffTime << "] " << PURPLE "<WebServ> "
				  << color << message << RESET << std::endl;
	else
		std::cout << CYAN "[" << buffTime << "] " << PURPLE "<WebServ> "
				  << color << message << RESET << std::endl;
}

void Logger::log(
	std::string const &level,
	std::string const &message,
	std::string const &color,
	bool const		   isError
)
{
	this->log(this->getLevel(level), message, color, isError);
}
