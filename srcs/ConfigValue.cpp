#include "ConfigValue.hpp"
#include "ServerException.hpp"

ConfigValue::ConfigValue(void) : type_(VECTOR) {}

ConfigValue::ConfigValue(std::vector<std::string> const &value)
	: type_(VECTOR), vectorValue_(value)
{
}

ConfigValue::ConfigValue(
	std::map<std::string, std::vector<std::string> > const &value
)
	: type_(MAP), mapValue_(value)
{
}

ConfigValue::ConfigValue(ConfigValue const &src)
{
	*this = src;
}

ConfigValue::~ConfigValue(void) {}

ConfigValue &ConfigValue::operator=(ConfigValue const &src)
{
	if (this != &src)
	{
		this->type_ = src.type_;
		this->vectorValue_ = src.vectorValue_;
		this->mapValue_ = src.mapValue_;
	}
	return *this;
}

ConfigValue::valueType ConfigValue::getType(void) const
{
	return this->type_;
}

// Returns the vector of strings stored in the ConfigValue object. If the type
// of ConfigValue is not a vector, it will throw an exception.
std::vector<std::string> const &ConfigValue::getVector(void) const
{
	if (this->type_ != VECTOR)
		throw ServerException("ConfigValue:getVector: not a vector.");
	return this->vectorValue_;
}

// Returns the string stored at index in the vector of strings stored in the
// ConfigValue object. If the type of ConfigValue is not a vector, it will throw
// an exception. If the index is out of bounds, it will return false.
bool ConfigValue::getVectorValue(unsigned int index, std::string &value) const
{
	if (this->type_ != VECTOR)
		throw ServerException(
			"ConfigValue::getVectorValue: not a vector, index:"
			+ std::to_string(index)
		);
	if (index >= this->vectorValue_.size())
		return false;
	value = this->vectorValue_[index];
	return true;
}

std::string	const &ConfigValue::getVectorValue(unsigned int index) const
{
	if (this->type_ != VECTOR)
		throw ServerException(
			"ConfigValue::getVectorValue: not a vector, index:"
			+ std::to_string(index)
		);
	if (index >= this->vectorValue_.size())
		throw ServerException(
			"ConfigValue::getVectorValue: index out of bounds, index:"
			+ std::to_string(index)
		);
	return this->vectorValue_[index];
}

// Returns the map of strings stored in the ConfigValue object. If the type of
// ConfigValue is not a map, it will throw an exception.
std::map<std::string, std::vector<std::string> > const &ConfigValue::getMap(void
) const
{
	if (this->type_ != MAP)
		throw ServerException("ConfigValue::getMap: not a map");
	return this->mapValue_;
}

// Returns the vector of strings stored at key in the map of strings stored in
// the ConfigValue object. If the type of ConfigValue is not a map, it will
// throw an exception. If the key is not found, it will return false.
bool ConfigValue::getMapValue(
	std::string const		 &key,
	std::vector<std::string> &value
) const
{
	if (this->type_ != MAP)
		throw ServerException(
			"ConfigValue::getMapValue: not a map, key:" + key
		);

	std::map<std::string, std::vector<std::string> >::const_iterator it;
	it = this->mapValue_.find(key);
	if (it == this->mapValue_.end())
		return false;
	value = it->second;
	return true;
}

std::vector<std::string> const &ConfigValue::getMapValue(std::string const &key) const
{
	if (this->type_ != MAP)
		throw ServerException(
			"ConfigValue::getMapValue: not a map, key:" + key
		);

	std::map<std::string, std::vector<std::string> >::const_iterator it;
	it = this->mapValue_.find(key);
	if (it == this->mapValue_.end())
		return it->second;
	return it->second;
}

void ConfigValue::setVector(std::vector<std::string> const &value)
{
	this->type_ = VECTOR;
	this->vectorValue_ = value;
}

void ConfigValue::setMap(
	std::map<std::string, std::vector<std::string> > const &value
)
{
	this->type_ = MAP;
	this->mapValue_ = value;
}

void ConfigValue::pushBackMapValue(
	std::string const &key,
	std::string const &value
)
{
	if (this->type_ != MAP)
		throw ServerException(
			"ConfigValue::pushBackMapValue: not a map, key:" + key
		);
	this->mapValue_[key].push_back(value);
}
