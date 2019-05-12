#ifndef VERSION_HPP
#define VERSION_HPP

#include <string>

namespace GBL {

struct Version
{
	uint64_t major = 17;
	uint64_t minor = 0;
	uint64_t patch = 0;
	uint64_t build = 0;

	struct Snapshot
	{
		uint64_t year;
		uint64_t week;
		uint64_t revision;
	} snapshot;

	enum class Type
	{
		Alpha,
		Beta,
		Snapshot,
		Pre,
		Release

	} prerel = Type::Release;

	bool required = false;

	std::string asString()
	{
		return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
	}
};  

extern Version version;

}

#endif