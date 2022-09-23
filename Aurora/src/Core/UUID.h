#pragma once

/*
 * Inside std::hash there is no need to return std::hash<uint64_t>{}(uuid) since std::hash returns a hash for non uniformly
 * distributed numbers and UUID are created with a std::uniform_distribution therefore we should just return it and hashing it
 * is redundant
 */

#include <xhash>

namespace Aurora {

	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		operator uint64_t() const { return m_Uuid; }

	private:
		uint64_t m_Uuid;
	};

	class UUID32
	{
	public:
		UUID32();
		UUID32(uint32_t uuid);
		UUID32(const UUID32&) = default;

		operator uint32_t() const { return m_Uuid; }

	private:
		uint32_t m_Uuid;
	};

}

namespace std {
	
	template<typename T> struct hash;

	template<>
	struct hash<Aurora::UUID>
	{
		std::size_t operator()(const Aurora::UUID& uuid) const
		{
			return (uint64_t)uuid;
		}
	};

	template<>
	struct hash<Aurora::UUID32>
	{
		std::size_t operator()(const Aurora::UUID32& uuid) const
		{
			return (uint32_t)uuid;
		}
	};

}