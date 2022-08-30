#include "Aurorapch.h"
#include "Random.h"

namespace Aurora {

	std::mt19937 Random::s_RandomEngine32;
	std::mt19937_64 Random::s_RandomEngine64;

	std::uniform_int_distribution<std::mt19937::result_type> Random::s_Distribution32;
	std::uniform_int_distribution<std::mt19937_64::result_type> Random::s_Distribution64;

}