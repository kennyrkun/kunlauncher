#ifndef VERSION_HPP
#define VERSION_HPP

#include <ostream>
#include <string>
#include <sstream>

namespace GBL {

namespace versions
{
	struct Snapshot
	{
		uint64_t year;
		uint64_t week;
		uint64_t revision;
	};

	// <3 https://sourcey.com/articles/comparing-version-strings-in-cpp
	struct Semantic
	{
	public:
		int major = 0;
		int minor = 0;
		int patch = 0;

		Semantic(int major, int minor, int patch) : major(major), minor(minor), patch(patch)
		{
		}

		std::string asString() const
		{
			std::stringstream ss;
			ss << major << "." << minor << "." << patch;
			return ss.str();
		}

		bool operator < (const Semantic& other)
		{
			if (versionCompare(this->asString(), other.asString()) == -1)
				return true;

			return false;
		}

		bool operator > (const Semantic& other)
		{
			if (versionCompare(this->asString(), other.asString()) == 1)
				return true;

			return false;
		}

		bool operator == (const Semantic& other)
		{
			return major == other.major
				&& minor == other.minor
				&& patch == other.patch;
		}

		friend std::ostream& operator << (std::ostream& stream, const Semantic& ver)
		{
			stream << ver.major;
			stream << '.';
			stream << ver.minor;
			stream << '.';
			stream << ver.patch;
			return stream;
		}

	private:
		int versionCompare(std::string v1, std::string v2)
		{
			//  vnum stores each numeric part of version 
			int vnum1 = 0, vnum2 = 0;

			//  loop untill both string are processed 
			for (int i = 0, j = 0; (i < v1.length() || j < v2.length()); )
			{
				//  storing numeric part of version 1 in vnum1 
				while (i < v1.length() && v1[i] != '.')
				{
					vnum1 = vnum1 * 10 + (v1[i] - '0');
					i++;
				}

				//  storing numeric part of version 2 in vnum2 
				while (j < v2.length() && v2[j] != '.')
				{
					vnum2 = vnum2 * 10 + (v2[j] - '0');
					j++;
				}

				if (vnum1 > vnum2)
					return 1;

				if (vnum2 > vnum1)
					return -1;

				//  if equal, reset variables and go for next numeric 
				// part 
				vnum1 = vnum2 = 0;
				i++;
				j++;
			}

			return 0;
		}
	};

	enum class ReleaseType
	{
		Alpha,
		Beta,
		Snapshot,
		Pre,
		Release
	};
}

class LauncherVersion : public versions::Semantic
{
public:
	LauncherVersion() : Semantic(17, 0, 0)
	{

	}

	versions::ReleaseType rtype = versions::ReleaseType::Release;

	bool required = false;
};  

extern LauncherVersion version;

}

#endif