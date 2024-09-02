#pragma once

#include <map>
#include <string>

// ServerInput class is used to parse the server input arguments and store them.
class ServerInput
{
  public:
	// Flags than can be set by user, using bitwise operations to store multiple
	// flags and to check them.
	typedef enum e_serverFlags
	{
		NONE = 0x00,
		V_LITE = 0x01,
		V_FULL = 0x02,
		HELP = 0x04,
		TEST = 0x08,
		TEST_PRINT = 0x10,
	} t_serverFlags;

	ServerInput();
	ServerInput(int argc, char **argv);
	ServerInput(ServerInput const &src);
	~ServerInput();

	ServerInput &operator=(ServerInput const &src);

	bool		hasThisFlag(t_serverFlags flag) const;
	std::string getHelpMessage(void) const;
	std::string getVersionMessage(void) const;
	std::string getFilePath(void) const;

  private:
	int										_flags;
	// Map to store all type of flags and their respective values.
	static std::map<std::string, int> const _flagMap;
	std::string								_filepath;

	void _parseArg(std::string const &arg, int index, int argc);
	void _setFlag(std::string const &flag);
	static std::map<std::string, int> const _createFlagMap(void);
};
