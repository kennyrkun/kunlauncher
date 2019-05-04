#ifndef VERSION_HPP
#define VERSION_HPP

#include <string>

namespace GBL {

struct Version
{
	uint64_t major = 17;
	uint64_t minor = 0;
	uint64_t patch = 0;

	// TODO: LauncherAPIVersion

	// enum beta, alpha, etc
	// uint release type

	uint64_t prerel = 0;

	uint64_t release;

	bool required = false;

	std::string asString()
	{
		return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
	}
};  

extern Version version;

}

#endif