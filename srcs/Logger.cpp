#include "Logger.hpp"
#include "colors.hpp"

#include <ctime>

// The static variable level_ is set to DEBUG by default.
Logger::Level Logger::level_ = Logger::DEBUG;
// The static variable _instance is the instance of the Logger class.
Logger Logger::instance_;

// Private constructor, the constructor is private to prevent the creation of
// multiple instances of the Logger class.
Logger::Logger(void) : currentLevel_(DEBUG), isError_(false){};

Logger::~Logger(void){};

void Logger::setLevel(Level level)
{
	level_ = level;
}

void Logger::setLevel(std::string const &level)
{
	if (level == "debug")
		level_ = Logger::DEBUG;
	else if (level == "info")
		level_ = Logger::INFO;
	else if (level == "error")
		level_ = Logger::ERROR;
}

// getLevel is a static function that returns the Level enum value of the static
// variable level_, that is the Logger current level.
Logger::Level Logger::getLevel(void)
{
	return level_; 
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
	else if (level == "error")
		return Logger::ERROR;
	return Logger::INFO;
}

std::string Logger::getLevel(Level const &level)
{
	 if (level == Logger::DEBUG)
    return "DEBUG";
  else if (level == Logger::INFO)
    return "INFO";
  else if (level == Logger::ERROR)
    return "ERROR";
  return "INFO";
}

// _prepareLog is a private function that prepares the log message with the
// header (current time + server) and set the level of the message.
void Logger::prepareLog_(Level const &level)
{
	this->currentLevel_ = level;

	time_t	   rawTime;
	struct tm *timeInfo;
	char	   buffTime[32];

	time(&rawTime);
	timeInfo = localtime(&rawTime);
	if (strftime(buffTime, sizeof(buffTime), "%T", timeInfo) == 0)
		buffTime[0] = '\0';

	std::string const color(this->getColor_(this->currentLevel_));
	std::string const strLevel(this->getLevel(this->currentLevel_));
	this->stream_ << CYAN "[" << buffTime << "] " << PURPLE "<WebServ> "
				  << color << "[" << strLevel << "] ";
}

std::string const Logger::getColor_(Level const &level) const
{
	if (level == Logger::DEBUG)
		return CYAN;
	else if (level == Logger::INFO)
		return WHITE;
	else if (level == Logger::ERROR)
		return RED;
	return BLUE;
}
