#include "utils.hpp"

#include <cstdlib>
#include <climits>

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

bool	isUint16(std::string const &str)
{
	if (str.length() > 5)
		return false;
	if (!isStrOfDigits(str))
		return false;
	char *end;
	unsigned long value = std::strtoul(str.c_str(), &end, 10);
	if (*end != '\0')
		return false;
	return value <= USHRT_MAX;
}

bool	strToUint16(std::string const &str, unsigned short &result)
{
	char *end;
	unsigned long value = std::strtoul(str.c_str(), &end, 10);

	if (*end != '\0' || end == str.c_str() || value > USHRT_MAX)
		return false;
	result = static_cast<unsigned short>(value);
	return true;
}

bool	isValidIPv4(std::string const &str)
{
	std::istringstream iss(str);
	std::string octet;
	int num, count = 0;

	while (std::getline(iss, octet, '.'))
	{
		if (octet.empty() || !isStrOfDigits(octet))
			return false;
		std::istringstream octetStream(octet);
		if (!(octetStream >> num) || num < 0 || num > 255)
			return false;
		++count;
	}
	return count == 4;
}

} // namespace ft
