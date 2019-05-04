////////////////////////////////////////////////////////////
// Copyright (c) 2016 Maximilian Wagenbach
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgement in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
////////////////////////////////////////////////////////////

// Revision 4

#ifndef SETTINGSPARSER_INCLUDE
#define SETTINGSPARSER_INCLUDE

#include <string>
#include <map>
#include <vector>
#include <sstream>

// TODO: I don't think `set` will set keys that don't already exist, this needs to be fixed.
// I think it's fixed in another version of this file in another project.

// TODO: if a boolean is set to 1 or 0 in the file it will be loaded as 0
// I don't know why this happens, but it should always load correctly.

// TODO: end keys if they have comments in them mid way

// TODO: clean up this file.
// there are so many conversions it's really nasty

class SettingsParser
{
public:
	SettingsParser();
	// constructs, loads, and parses file at filename
	SettingsParser(const std::string& filename);
	~SettingsParser();

	// loads and parses a file at filename
	bool loadFromFile(const std::string& filename);

	// Gets the key from the file.
	// Writes key into T at value.
	template<typename T>
	bool get(const std::string& key, T& value) const;

	// returns a value of T for key
	template<typename T>
	T get(const std::string& key) const;

	// This method tries to read the value of a key into a vector. The values have to be
	// seperated by comma. The vector is cleared before it is filled.
	template<typename T>
	bool get(const std::string& key, std::vector<T> &value) const;

	// reads the value of a key into a vector and returns it directly
	template<typename T>
	std::vector<T> get(const std::string& key) const;

	// reads the value of K into value of type T
	template<typename T>
	void set(const std::string &key, const T value);

	// reads a vector of values into a vector of Ts at value
	template<typename T>
	void set(const std::string& key, const std::vector<T> value);

	// Prints the contents of the file.
	void print() const;

private:
	//return the string in the type of T
	template<typename T>
	T convertToType(const std::string &input) const;
	//return string of type T
	template<typename T>
	std::string convertToStr(const T input) const;

	bool read();
	bool write() const;
	std::pair<std::string, std::string> parseLine(const std::string &line) const;

	bool m_isChanged;
	std::string m_filename;
	std::map<std::string, std::string> m_data;
	const std::locale m_locale;
};

// TODO: do we need this?
// this seems like it'll throw a runtime exception, which could be bad.
// it would be better if this wasn't here and we got a compile error instead.
template<typename T>
inline std::string SettingsParser::convertToStr(T value) const 
{
	try
	{
		std::string str;
		std::stringstream ss(value);
		ss << str;

		return str;
	}
	catch (const std::exception& e)
	{
		std::cerr << "[SettingsParser] ERROR: " << e.what() << std::endl;
		throw "Unsupported type supplied, either change types, or write a correct conversion function for the template type.";
	}
}

template<>
inline std::string SettingsParser::convertToStr<bool>(bool value) const
{
	return (value) ? "TRUE" : "FALSE";
}

template<>
inline std::string SettingsParser::convertToStr<std::string>(std::string value) const 
{
	return value;
}

template<>
inline std::string SettingsParser::convertToStr<const char*>(const char* value) const 
{
	return std::string(value);
}

template<>
inline std::string SettingsParser::convertToStr<char>(char value) const 
{
	std::string tmp = "";
	tmp = value;
	return tmp;
}

template<>
inline std::string SettingsParser::convertToStr<int>(int value) const 
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

template<>
inline std::string SettingsParser::convertToStr<long>(long value) const
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

template<>
inline std::string SettingsParser::convertToStr<long long>(long long value) const
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

template<>
inline std::string SettingsParser::convertToStr<float>(float value) const 
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

template<>
inline std::string SettingsParser::convertToStr<short>(short value) const 
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

template<>
inline std::string SettingsParser::convertToStr<double>(double value) const 
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

template <typename T>
inline T SettingsParser::convertToType(const std::string &input) const 
{
	try
	{
		T value;
		std::stringstream ss(input);
		ss >> value;

		return value;
	}
	catch (const std::exception& e)
	{
		std::cerr << "[SettingsParser] ERROR: " << e.what() << std::endl;
		throw "Unconvertable type encountered, please use a different type, or define the handle case in SettingsParser.hpp";
	}
}

template<>
inline bool SettingsParser::convertToType<bool>(const std::string &input) const
{
	return input == "TRUE" ? true : false;
}

template<>
inline int SettingsParser::convertToType<int>(const std::string &input) const 
{
	int value;
	std::stringstream ss(input);
	ss >> value;

	return value;
}

template<>
inline long SettingsParser::convertToType<long>(const std::string &input) const
{
	long value;
	std::stringstream ss(input);
	ss >> value;

	return value;
}

template<>
inline long long SettingsParser::convertToType<long long>(const std::string &input) const
{
	long long value;
	std::stringstream ss(input);
	ss >> value;

	return value;
}

template<>
inline unsigned int SettingsParser::convertToType<unsigned int>(const std::string &input) const 
{
	unsigned int value;
	std::stringstream ss(input);
	ss >> value;

	return value;
}

template<>
inline double SettingsParser::convertToType<double>(const std::string &input) const 
{
	double value;
	std::stringstream ss(input);
	ss >> value;

	return value;
}

template<>
inline float SettingsParser::convertToType<float>(const std::string &input) const 
{
	float value;
	std::stringstream ss(input);
	ss >> value;

	return value;
}

template<>
inline short SettingsParser::convertToType<short>(const std::string &input) const 
{
	short value;
	std::stringstream ss(input);
	ss >> value;

	return value;
}

template<>
inline char SettingsParser::convertToType<char>(const std::string &input) const 
{
	return input[0];
}

template<>
inline std::string SettingsParser::convertToType<std::string>(const std::string &input) const 
{
	return input;
}

template<typename T>
// overwrites value with T from key
inline bool SettingsParser::get(const std::string& key, T &value) const 
{
	auto it = m_data.find(key);

	if (it != m_data.end()) {
		value = convertToType<T>(it->second);

		return true;
	}
	else
	{
		std::cerr << "failed to find key \"" << key << "\"" << std::endl;

		return false;
	}
}

template<typename T>
// TODO: this doesn't work for some reason
// directly returns the value of key
inline T SettingsParser::get(const std::string& key) const 
{
	auto it = m_data.find(key);

	if (it != m_data.end())
		return convertToType<T>(it->second);
	else
		std::cerr << "failed to find key \"" << key << "\"" << std::endl;
}

template<typename T>
// writes the result into vector value
inline bool SettingsParser::get(const std::string& key, std::vector<T> &value) const 
{
	auto it = m_data.find(key);

	if (it != m_data.end()) 
	{

		std::string output;
		std::istringstream parser(it->second);

		value.clear();

		//split by comma
		while (getline(parser, output, ',')) 
		{
			value.push_back(convertToType<T>(output));
		}

		return true;
	}
	else
	{
		std::cerr << "failed to find key \"" << key << "\"" << std::endl;

		return false;
	}
}

template<typename T>
// returns a completely new vector of T
inline std::vector<T> SettingsParser::get(const std::string& key) const 
{
	auto it = m_data.find(key);

	if (it != m_data.end()) 
	{
		std::vector<T> value;

		std::string output;
		std::istringstream parser(it->second);

		value.clear();

		//split by comma
		while (getline(parser, output, ',')) 
		{
			value.push_back(convertToType<T>(output));
		}

		return value;
	}
	else
	{
		std::cerr << "failed to find key \"" << key << "\"" << std::endl;
	}
}

template<typename T>
inline void SettingsParser::set(const std::string& key, const T value) 
{
	// the [] operator replaces the value if the key is found, if not it creates a new element
	std::string v = convertToStr<T>(value);

	if (!v.empty())
		m_data[key] = convertToStr<T>(value);
	else
		m_data.erase(key);

	write();
}

template<typename T>
// sets a vector with key, separated by commas
// (spaces are included: 1,2,3 or will be strings
inline void SettingsParser::set(const std::string &key, const std::vector<T> value) 
{
	// transform the vector into a string that seperates the elements with a comma
	std::string valueAsString;
	for (size_t i = 0; i < value.size() - 1; ++i) 
	{
		valueAsString += convertToStr<T>(value.at(i)) + ",";
	}
	valueAsString += convertToStr<T>(value.back());

	// the [] operator replaces the value if the key is found, if not it creates a new element
	m_data[key] = valueAsString;
	m_isChanged = true;
}

#endif // SETTINGSPARSER_INCLUDE