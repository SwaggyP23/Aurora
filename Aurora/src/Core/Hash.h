#pragma once

#include <string_view>

namespace Aurora {

	class Hash
	{
	public:
		// Refernce: https://gist.github.com/hwei/1950649d523afd03285c
		static constexpr uint32_t GenerateFNVHash(std::string_view str)
		{
			constexpr uint32_t FNV_PRIME = 16777619u;
			constexpr uint32_t OFFSET_BASIS = 2166136261u;

			size_t hash = OFFSET_BASIS;
			size_t length = str.length();
			const char* data = str.data();

			for (size_t i = 0; i < length; i++)
			{
				hash ^= *data++;
				hash *= FNV_PRIME;
			}

			return (uint32_t)hash;
		}
	};

}