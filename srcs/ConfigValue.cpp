#include "ConfigValue.hpp"

ConfigValue::ConfigValue(void) : _type(VECTOR) {}

ConfigValue::ConfigValue(std::vector<std::string> const &value)
	: _type(VECTOR), _vectorValue(value)
{
}

ConfigValue::ConfigValue(
	std::map<std::string, std::vector<std::string> > const &value
)
	: _type(MAP), _mapValue(value)
{
}

ConfigValue::valueType ConfigValue::getType(void) const
{
	return this->_type;
}

std::vector<std::string> const &ConfigValue::getVector(void) const
{
	return this->_vectorValue;
}

bool ConfigValue::getVectorValue(unsigned int index, std::string &value) const
{
	if (index >= this->_vectorValue.size())
		return false;
	value = this->_vectorValue[index];
	return true;
}

std::map<std::string, std::vector<std::string> > const &ConfigValue::getMap(void
) const
{
	return this->_mapValue;
}

bool	ConfigValue::getMapValue(
	std::string const &key,
	std::vector<std::string> &value
) const
{
	std::map<std::string, std::vector<std::string> >::const_iterator it;
	it = this->_mapValue.find(key);
	if (it == this->_mapValue.end())
		return false;
	value = it->second;
	return true;
}

void ConfigValue::setVector(std::vector<std::string> const &value)
{
	this->_type = VECTOR;
	this->_vectorValue = value;
}

void ConfigValue::setMap(
	std::map<std::string, std::vector<std::string> > const &value
)
{
	this->_type = MAP;
	this->_mapValue = value;
}
