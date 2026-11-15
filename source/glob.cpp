#include <cstdint>
#include <functional>
#include <vector>

#include "glob.h"

namespace {
	std::string NormalizePath(const std::string& path)
	{
		std::string out = path;

		for (char& c : out)
			if (c == '\\')
				c = '/';

		return out;
	}

	bool MatchGlob(const std::string& pattern, const std::string value)
	{
		const size_t patternSize = pattern.size();
		const size_t valueSize = value.size();

		std::vector<std::uint8_t> cache((patternSize + 1) * (valueSize + 1), -1);
		std::function<bool(size_t, size_t)> match = [&](size_t p, size_t s) -> bool {
			const size_t index = p * (valueSize + 1) + s;

			bool result = false;

			// End of pattern.
			if (p == patternSize)
			{
				result = (s == valueSize);
			}
			else if (pattern[p] == '*')
			{
				// check for double wildcard '**'
				const bool recursive = p + 1 < patternSize && pattern[p + 1] == '*';

				if (recursive)
				{
					// collapse consecutive stars
					size_t next = p + 2;

					while (next < patternSize && pattern[next] == '*')
						++next;

					// '**/' matches zero or more directories
					if (next < patternSize && pattern[next] == '/')
					{
						// match zero directories
						result = match(next + 1, s);

						// or consume one directory at a time
						if (!result)
						{
							for (size_t i = s; i < valueSize; ++i)
							{
								if (value[i] == '/' && match(p, i + 1))
								{
									result = true;
									break;
								}
							}
						}
					}
					else
					{
						// ** matches any characters including '/'
						result = match(next, s);

						if (!result && s < valueSize)
							result = match(p, s + 1);
					}
				}
				else
				{
					// * matches zero or more non-separator characters
					result = match(p + 1, s);

					if (!result && s < valueSize && value[s] != '/')
						result = match(p, s + 1);
				}
			}
			else if (pattern[p] == '?')
			{
				// ? matches exactly one non-separator character
				result = s < valueSize && value[s] != '/' && match(p + 1, s + 1);
			}
			else
			{
				// literal character
				result = s < valueSize && pattern[p] == value[s] && match(p + 1, s + 1);
			}

			cache[index] = result ? 1 : 0;
			return result;
		};

		return match(0, 0);
	}
}

//

bool clsp::glob::Match(const std::string& pattern, const std::string& value)
{
	return MatchGlob(NormalizePath(pattern), NormalizePath(value));
}