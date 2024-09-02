#include "ConfigValue.hpp"
#include "ServerException.hpp"

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

// Returns the vector of strings stored in the ConfigValue object. If the type
// of ConfigValue is not a vector, it will throw an exception.
std::vector<std::string> const &ConfigValue::getVector(void) const
{
	if (this->_type != VECTOR)
		throw ServerException("ConfigValue:getVector: not a vector.");
	return this->_vectorValue;
}

// Returns the string stored at index in the vector of strings stored in the
// ConfigValue object. If the type of ConfigValue is not a vector, it will throw
// an exception. If the index is out of bounds, it will return false.
bool ConfigValue::getVectorValue(unsigned int index, std::string &value) const
{
	if (this->_type != VECTOR)
		throw ServerException(
			"ConfigValue::getVectorValue: not a vector, index:" +
			std::to_string(index)
		);
	if (index >= this->_vectorValue.size())
		return false;
	value = this->_vectorValue[index];
	return true;
}

// Returns the map of strings stored in the ConfigValue object. If the type of
// ConfigValue is not a map, it will throw an exception.
std::map<std::string, std::vector<std::string> > const &ConfigValue::getMap(void
) const
{
	if (this->_type != MAP)
		throw ServerException("ConfigValue::getMap: not a map");
	return this->_mapValue;
}

// Returns the vector of strings stored at key in the map of strings stored in
// the ConfigValue object. If the type of ConfigValue is not a map, it will
// throw an exception. If the key is not found, it will return false.
bool ConfigValue::getMapValue(
	std::string const		 &key,
	std::vector<std::string> &value
) const
{
	if (this->_type != MAP)
		throw ServerException(
			"ConfigValue::getMapValue: not a map, key:" + key
		);

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
