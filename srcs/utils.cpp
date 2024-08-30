#include "utils.hpp"

namespace ft
{

std::string &trim(std::string &str, std::string const &delimiters)
{
	str.erase(0, str.find_first_not_of(delimiters));
	str.erase(str.find_last_not_of(delimiters) + 1);
	return str;
}

template <> std::string toString<bool>(bool const &value)
{
	return value ? "true" : "false";
}

std::vector<std::string> &split(
	std::vector<std::string> &result,
	std::string const		 &str,
	std::string const		 &delimiters
)
{
	size_t start = 0;
	size_t end = str.find_first_of(delimiters);
	while (end != std::string::npos)
	{
		if (end != start)
			result.push_back(str.substr(start, end - start));
		start = end + 1;
		end = str.find_first_of(delimiters, start);
	}
	if (start < str.length())
		result.push_back(str.substr(start));
	return result;
}

bool	isStrOfDigits(std::string const &str)
{
	return str.find_first_not_of("0123456789") == std::string::npos;
}

} // namespace ft
