#pragma once

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
			return hash<uint64_t>{}((uint64_t)uuid);
		}
	};

	template<>
	struct hash<Aurora::UUID32>
	{
		std::size_t operator()(const Aurora::UUID32& uuid) const
		{
			return hash<uint32_t>{}((uint32_t)uuid);
		}
	};

}