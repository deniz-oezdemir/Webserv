#include "Logger.hpp"
#include "colors.hpp"

#include <ctime>

// The static variable _level is set to DEBUG by default.
Logger::Level Logger::_level = Logger::DEBUG;
// The static variable _instance is the instance of the Logger class.
Logger Logger::_instance;

// Private constructor, the constructor is private to prevent the creation of
// multiple instances of the Logger class.
Logger::Logger(void) : _currentLevel(DEBUG), _isError(false){};

Logger::~Logger(void){};

void Logger::setLevel(Level level)
{
	_level = level;
}

void Logger::setLevel(std::string const &level)
{
	if (level == "debug")
		_level = Logger::DEBUG;
	else if (level == "info")
		_level = Logger::INFO;
	else if (level == "warm")
		_level = Logger::WARN;
	else if (level == "error")
		_level = Logger::ERROR;
}

// getLevel is a static function that returns the Level enum value of the static
// variable _level, that is the Logger current level.
Logger::Level Logger::getLevel(void)
{
	return _level;
}

// getLevel is a static function that returns the Level enum value of the string
// passed as a parameter. If the string is not a valid level, it will return
// INFO.
Logger::Level Logger::getLevel(std::string const &level)
{
	if (level == "debug")
		return Logger::DEBUG;
	else if (level == "info")
		return Logger::INFO;
	else if (level == "warn")
		return Logger::WARN;
	else if (level == "error")
		return Logger::ERROR;
	return Logger::INFO;
}

// _prepareLog is a private function that prepares the log message with the
// header (current time + server) and set the level of the message.
void Logger::_prepareLog(Level const &level)
{
	this->_currentLevel = level;

	time_t	   rawTime;
	struct tm *timeInfo;
	char	   buffTime[32];

	time(&rawTime);
	timeInfo = localtime(&rawTime);
	if (strftime(buffTime, sizeof(buffTime), "%T", timeInfo) == 0)
		buffTime[0] = '\0';

	std::string const color(this->_getColor(this->_currentLevel));
	this->_stream << CYAN "[" << buffTime << "] " << PURPLE "<WebServ> "
				  << color;
}

std::string const Logger::_getColor(Level const &level) const
{
	if (level == Logger::DEBUG)
		return CYAN;
	else if (level == Logger::INFO)
		return WHITE;
	else if (level == Logger::WARN)
		return YELLOW;
	else if (level == Logger::ERROR)
		return RED;
	return BLUE;
}
