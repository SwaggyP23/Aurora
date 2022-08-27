#include "Aurorapch.h"
#include "UUID.h"

#include <random>

namespace Aurora {

	static std::random_device s_RandomDevice;
	static std::mt19937_64 s_Engine64(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDistrib64;

	static std::mt19937 s_Engine32(s_RandomDevice());
	static std::uniform_int_distribution<uint32_t> s_UniformDistrib32;

	UUID::UUID()
		: m_Uuid(s_UniformDistrib64(s_Engine64))
	{
	}

	UUID::UUID(uint64_t uuid)
		: m_Uuid(uuid)
	{
	}

	UUID32::UUID32()
		: m_Uuid(s_UniformDistrib32(s_Engine32))
	{
	}

	UUID32::UUID32(uint32_t uuid)
		: m_Uuid(uuid)
	{
	}

}