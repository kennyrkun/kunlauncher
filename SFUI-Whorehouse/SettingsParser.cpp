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

#include "SettingsParser.hpp"

#include <locale>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>


SettingsParser::SettingsParser() : m_isChanged(false)
{

}


SettingsParser::~SettingsParser()
{
	saveToFile();
}


bool SettingsParser::loadFromFile(const std::string& filename)
{
	m_data.clear();
	m_filename = filename;
	return read();
}


bool SettingsParser::saveToFile()
{
	if (m_isChanged)
	{
		m_isChanged = false;
		return write();
	}
	return true;
}


bool SettingsParser::read()
{
	std::ifstream in(m_filename);
	if (!in.is_open())
	{
		std::cerr << "unable to open settings file \"" << m_filename << "\" for reading!" << "\n";
		return false;
	}

	std::string line;
	while (std::getline(in, line))
	{
		// parse line
		std::pair<std::string, std::string> keyValuePair = parseLine(line);

		if (!keyValuePair.first.empty())
		{
			// if the line is not empty or a comment save it to the map
			m_data[keyValuePair.first] = keyValuePair.second;
		}
	}

	in.close();
	m_isChanged = false;
	return true;
}


bool SettingsParser::write() const
{
	std::vector<std::pair<std::string, std::string>> fileContents;

	std::ifstream in(m_filename);

	// read the file into a vector and replace the values of the keys that match with our map
	if (in.is_open())
	{
		std::string line;
		while (std::getline(in, line))
		{
			// parse line
			std::pair<std::string, std::string> keyValuePair = parseLine(line);

			if (!keyValuePair.first.empty())
			{
				// check if the key is found in the map
				auto it = m_data.find(keyValuePair.first);
				if (it != m_data.end())
				{
					// if so take it's value, otherwise the value from the file is kept
					keyValuePair.second = it->second;
				}
			}
			else
			{
				// if the line is empty or a comment simply take the whole line as the key
				keyValuePair.first = line;
			}
			fileContents.push_back(keyValuePair);
		}
	}
	else
	{
		// Can't open file for reading. Use only the data from the map
		for (auto it = m_data.begin(); it != m_data.end(); ++it)
			fileContents.push_back(std::make_pair(it->first, it->second));
	}

	in.close();


	// open the file for writing
	std::ofstream out(m_filename);
	if (!out.is_open())
	{
		std::cerr << "unable to open settings file \"" << m_filename << "\" for writing!" << "\n";
		return false;
	}
	for (auto it = fileContents.begin(); it != fileContents.end(); ++it)
	{
		out << it->first; // write the key

		if (!it->second.empty())
			// if this line is not empty or a comment also write the assignment and the value
			out << " = " << it->second;

		out << "\n";
	}
	out.close();
	return true;
}


/**
* This method parses a line from our format ("key = value") into a std::pair<std::string, std::string>
* containing the key and the value.
* If the line is empty or a comment (starts with a '#') an empty pair is returned.
*/
std::pair<std::string, std::string> SettingsParser::parseLine(const std::string &line) const
{
	if (line.size() > 0 && (line[0] != '/' && line[1] != '/'))
	{
		size_t index = 0;
		// trim leading whitespace
		while (std::isspace(line[index], m_locale))
			index++;
		// get the key string
		const size_t beginKeyString = index;
		while (!std::isspace(line[index], m_locale) && line[index] != '=')
			index++;
		const std::string key = line.substr(beginKeyString, index - beginKeyString);

		// skip the assignment
		while (std::isspace(line[index], m_locale) || line[index] == '=')
			index++;

		// get the value string
		const std::string value = line.substr(index, line.size() - index);

		// return the key value pair
		return std::make_pair(key, value);
	}

	// if this line is emtpy or a comment, return an empty pair
	return std::make_pair(std::string(), std::string());
}


void SettingsParser::print() const
{
	for (auto& element : m_data)
		std::cout << element.first << " = " << element.second << "\n";

	std::cout << std::endl << "Size: " << m_data.size() << "\n";
}


bool SettingsParser::isChanged() const
{
	return m_isChanged;
}