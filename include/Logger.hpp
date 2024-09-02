#pragma once

#include <string>

// Logger class is used to log messages to the console. Depending on the level
// set, it will log the message or not.
class Logger
{
  public:
	enum Level
	{
		DEBUG,
		INFO,
		WARN,
		ERROR,
	};

	Logger(void);
	~Logger(void);

	void  setLevel(Level level);
	void  setLevel(std::string const &level);
	Level getLevel(void) const;
	Level getLevel(std::string const &level) const;

	void
	log(Level const		   level,
		std::string const &message,
		std::string const &color,
		bool const		   isError = false);
	void
	log(std::string const &level,
		std::string const &message,
		std::string const &color,
		bool const		   isError = false);

  private:
	Logger(const Logger &src);
	Logger &operator=(const Logger &rhs);

	Level _level;
};
