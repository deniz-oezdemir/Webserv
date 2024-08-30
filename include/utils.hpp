#pragma once

#include <sstream>
#include <string>
#include <vector>

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
bool	isStrOfDigits(std::string const &str);

} // namespace ft
