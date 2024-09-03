#pragma once

#include "colors.hpp"
#include <iostream>
#include <sstream>
#include <string>

// INFO: Logger class is used to log messages to the console. Depending on the level
// set, it will log the message or not.
// WARNING: Always you should finish the "<<" overload with std::endl or
// std::flush, and please start the message with a capital letter.
// Example:
// To use cout:
// Logger::Log(Logger::INFO) << "Some text" << someVar << std::endl;
// To use cerr:
// Logger::Log(Logger::ERROR, true) << "Some text" << someVar << std::endl;
// NOTE: Logging levels:
// [DEBUG] For debugging purposes to track the flow of the program.
// [INFO] For general information about the program, useful to the user.
// [ERROR] For errors in the server, a function crash, or a file not found, etc
class Logger
{
  public:
	enum Level
	{
		DEBUG,
		INFO,
		ERROR,
	};

	// Setters and getters, with string or Level type as parameter.
	static void		   setLevel(Level level);
	static void		   setLevel(std::string const &level);
	static Level	   getLevel(void);
	static Level	   getLevel(std::string const &level);
	static std::string getLevel(Level const &level);

	static Logger &log(Level const level, bool isError = false)
	{
		_instance._prepareLog(level);
		_instance._isError = isError;
		return _instance;
	}

	template <typename T> Logger &operator<<(T const &message)
	{
		this->_stream << message;
		return *this;
	}

	Logger &operator<<(std::ostream &(*os)(std::ostream &))
	{
		if (this->_currentLevel >= this->_level)
		{
			this->_stream << RESET;
			os(this->_stream);
			if (this->_isError)
				std::cerr << this->_stream.str();
			else
				std::cout << this->_stream.str();
		}
		this->_stream.str("");
		this->_stream.clear();
		return *this;
	}

	static void
	log(Level const		   level,
		std::string const &message,
		bool const		   isError = false);
	static void
	log(std::string const &level,
		std::string const &message,
		bool const		   isError = false);

  private:
	static Level	   _level;
	static Logger	   _instance;
	std::ostringstream _stream;
	Level			   _currentLevel;
	bool			   _isError;

	Logger(void);
	Logger(const Logger &src);
	Logger &operator=(const Logger &rhs);
	~Logger(void);

	void			  _prepareLog(Level const &level);
	std::string const _getColor(Level const &level) const;
};
