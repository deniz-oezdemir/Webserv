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
		instance_.prepareLog_(level);
		instance_.isError_ = isError;
		return instance_;
	}

	template <typename T> Logger &operator<<(T const &message)
	{
		this->stream_ << message;
		return *this;
	}

	Logger &operator<<(std::ostream &(*os)(std::ostream &))
	{
		if (this->currentLevel_ >= this->level_)
		{
			this->stream_ << RESET;
			os(this->stream_);
			if (this->isError_)
				std::cerr << this->stream_.str();
			else
				std::cout << this->stream_.str();
		}
		this->stream_.str("");
		this->stream_.clear();
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
	static Level	   level_;
	static Logger	   instance_;
	std::ostringstream stream_;
	Level			   currentLevel_;
	bool			   isError_;

	Logger(void);
	Logger(const Logger &src);
	Logger &operator=(const Logger &rhs);
	~Logger(void);

	void			  prepareLog_(Level const &level);
	std::string const getColor_(Level const &level) const;
};
