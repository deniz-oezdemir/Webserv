#include "ServerInput.hpp"
#include "ServerException.hpp"
#include "colors.hpp"

#include <sstream>

std::map<std::string, int> const ServerInput::flagMap_ =
	ServerInput::createFlagMap_();

std::map<std::string, int> const ServerInput::createFlagMap_()
{
	std::map<std::string, int> m;
	m["--version"] = V_LITE;
	m["-v"] = V_LITE;
	m["--Version"] = V_FULL;
	m["-V"] = V_FULL;
	m["--help"] = HELP;
	m["-h"] = HELP;
	m["-H"] = HELP;
	m["-?"] = HELP;
	m["--test"] = TEST;
	m["-t"] = TEST;
	m["--Test"] = TEST_PRINT;
	m["-T"] = TEST_PRINT;
	return m;
}

ServerInput::ServerInput()
	: flags_(this->NONE), filepath_("./default.config"){};

ServerInput::ServerInput(int argc, char **argv)
	: flags_(this->NONE), filepath_("./default.config")
{
	for (int i = 1; i < argc; ++i)
		this->parseArg_(argv[i], i, argc);
};

ServerInput::ServerInput(ServerInput const &src)
{
	*this = src;
};

ServerInput &ServerInput::operator=(ServerInput const &src)
{
	if (this != &src)
	{
		this->flags_ = src.flags_;
		this->filepath_ = src.filepath_;
	}
	return *this;
};

ServerInput::~ServerInput(){};

// Parse the argument, if it is a flag, set the flag, if it is the last argument
// and not a flag, set the filepath.
void ServerInput::parseArg_(std::string const &arg, int index, int argc)
{
	if (arg[0] == '-')
		this->setFlag_(arg);
	else if (index == argc - 1)
		this->filepath_ = arg;
	else
		throw ServerException(
			"Invalid argument=> %\n\n" + this->getHelpMessage(), 0, arg
		);
};

// Set the flag, if it is valid use OR bitwise operation to store the respective
// bit in flags_. Otherwise, throw an exception.
void ServerInput::setFlag_(std::string const &flag)
{
	std::map<std::string, int>::const_iterator it = this->flagMap_.find(flag);
	if (it != this->flagMap_.end())
		this->flags_ |= it->second;
	else
		throw ServerException(
			"Invalid flag=> %\n\n" + this->getHelpMessage(), 0, flag
		);
};

// Check if the flag is set using AND bitwise operation.
bool ServerInput::hasThisFlag(t_serverFlags flag) const
{
	return (this->flags_ & flag);
};

std::string ServerInput::getHelpMessage(void) const
{
	return YELLOW BOLD "Usage:\n" CYAN "\t./webserv " WHITE BOLD
					   "[OPTIONAL: -flag][OPTIONAL: config_file]\n" YELLOW BOLD
					   "Flags:\n" CYAN BOLD "\t-h, -H, -?" RESET WHITE
					   "\t\tDisplay this help message\n" CYAN BOLD
					   "\t-v, --version" RESET WHITE
					   "\t\tDisplay version information\n" CYAN BOLD
					   "\t-V, --Version" RESET WHITE
					   "\t\tDisplay full version information\n" CYAN BOLD
					   "\t-t, --test" RESET WHITE
					   "\t\tCheck the configuration file\n" CYAN BOLD
					   "\t-T, --Test" RESET WHITE
					   "\t\tCheck the configutation file and print it\n" RESET;
}

std::string ServerInput::getVersionMessage(void) const
{
	std::stringstream ss;

	ss << YELLOW BOLD "WebServ " << RESET ULINE CYAN "v0.0.1" << "\n" RESET;
	if (this->hasThisFlag(ServerInput::V_LITE))
		return ss.str();

#ifdef __clang__
	ss << WHITE "Compiled with" << YELLOW " Clang "
	   << WHITE "version " CYAN BOLD << __clang_major__ << "."
	   << __clang_minor__ << "." << __clang_patchlevel__ << RESET;
#elif defined(__GNUC__)
	ss << WHITE "Compiled with " << YELLOW " GCC " << WHITE " version "
	   << CYAN ULINE << __GNUC__ << "." << __GNUC_MINOR__ << "."
	   << __GNUC_PATCHLEVEL__ << RESET;
#elif defined(_MSC_VER)
	ss << WHITE "Compiled with " << YELLOW "MSVC " << WHITE "version " CYAN BOLD
	   << _MSC_VER << RESET;
#else
	ss << RED "Unknown compiler" << RESET;
#endif

	ss << WHITE "\nConfiguration file path: " YELLOW << this->filepath_ << '\n';
	ss << WHITE "Created by " << CYAN BOLD "[johnavar] " << "[jmigoya] "
	   << "[denizozd] " << RESET;

	return ss.str();
}

std::string ServerInput::getFilePath(void) const
{
	return this->filepath_;
}
