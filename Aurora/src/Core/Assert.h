#pragma once

#ifdef AURORA_DEBUG

// TODO: Change to use a logging function called printassertmessage made especially for asserts
#define AR_CORE_ASSERT_INTERNAL(...)   ::Aurora::Logger::Log::PrintAssertMessageWithTag(::Aurora::Logger::Log::Type::Core, "CORE", "Assertion failed!", __VA_ARGS__)
#define AR_ASSERT_INTERNAL(...)        ::Aurora::Logger::Log::PrintAssertMessageWithTag(::Aurora::Logger::Log::Type::Client, "CLIENT", "Assertion failed!", __VA_ARGS__)

#define AR_CORE_ASSERT(check, ...)     { if(!(check)) { AR_CORE_ASSERT_INTERNAL(__VA_ARGS__); AR_DEBUG_BREAK; } }
#define AR_ASSERT(check, ...)          { if(!(check)) { AR_ASSERT_INTERNAL(__VA_ARGS__); AR_DEBUG_BREAK; } }

#else

#define AR_CORE_ASSERT(check, ...)
#define AR_ASSERT(check, ...)

#endif