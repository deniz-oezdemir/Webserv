#pragma once

#include <sstream>
#include <string>
#include <vector>

// Fortytwo namespace contains utility functions.
namespace ft
{

template <typename T> std::string toString(T const &value)
{
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

template <> std::string toString<bool>(bool const &value);

std::string &trim(std::string &str, std::string const &chars = "\t\n\v\f\r ");
std::vector<std::string> &split(
	std::vector<std::string> &result,
	std::string const		 &str,
	std::string const		 &delimiters = " \t\n"
);
bool isStrOfDigits(std::string const &str);
// isUint16 checks if a string is a valid unsigned short, useful to check the
// port number.
bool isUint16(std::string const &str);
// strToUint16 converts a string to an unsigned short..
bool strToUint16(std::string const &str, unsigned short &value);
// isValidIPv4 checks if a string is a valid IPv4 address.
bool isValidIPv4(std::string const &str);

} // namespace ft
