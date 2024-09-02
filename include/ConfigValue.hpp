#pragma once

#include <map>
#include <string>
#include <vector>

// ConfigValue class is used to store the value of a configuration key
// (directive) in the servers configuation map inside ServerConfig class. It can
// store a vector of strings or a map of strings, depending on the value type.
class ConfigValue
{
  public:
	enum valueType
	{
		VECTOR,
		MAP
	};

	ConfigValue(void);
	ConfigValue(std::vector<std::string> const &value);
	ConfigValue(std::map<std::string, std::vector<std::string> > const &value);

	valueType						getType(void) const;
	std::vector<std::string> const &getVector(void) const;
	bool getVectorValue(unsigned int index, std::string &value) const;
	std::map<std::string, std::vector<std::string> > const &getMap(void) const;
	bool
	getMapValue(std::string const &key, std::vector<std::string> &value) const;
	void setVector(std::vector<std::string> const &value);
	void setMap(std::map<std::string, std::vector<std::string> > const &value);

  private:
	valueType										_type;
	std::vector<std::string>						_vectorValue;
	std::map<std::string, std::vector<std::string> > _mapValue;
};
