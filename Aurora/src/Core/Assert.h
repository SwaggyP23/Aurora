#pragma once

#ifdef AURORA_DEBUG

    #define AR_CORE_ASSERT_INTERNAL(...)   ::Aurora::Logger::Log::PrintAssertMessageWithTag(::Aurora::Logger::Log::Type::Core, "CORE", "Assertion failed!", __VA_ARGS__)
    #define AR_ASSERT_INTERNAL(...)        ::Aurora::Logger::Log::PrintAssertMessageWithTag(::Aurora::Logger::Log::Type::Client, "CLIENT", "Assertion failed!", __VA_ARGS__)

    #define AR_CORE_ASSERT(check, ...)     { if(!(check)) { AR_CORE_ASSERT_INTERNAL(__VA_ARGS__); AR_DEBUG_BREAK(); } }
    #define AR_ASSERT(check, ...)          { if(!(check)) { AR_ASSERT_INTERNAL(__VA_ARGS__); AR_DEBUG_BREAK(); } }

#else

    #define AR_CORE_ASSERT(check, ...)
    #define AR_ASSERT(check, ...)

#endif

#ifndef AURORA_DIST

    #define AR_CORE_CHECK_INTERNAL(...)    ::Aurora::Logger::Log::PrintAssertMessageWithTag(::Aurora::Logger::Log::Type::Core, "CORE", "Check failed!", __VA_ARGS__)
    #define AR_CHECK_INTERNAL(...)         ::Aurora::Logger::Log::PrintAssertMessageWithTag(::Aurora::Logger::Log::Type::Client, "CLIENT", "Check failed!", __VA_ARGS__)

    #define AR_CORE_CHECK(check, ...)      { if(!(check)) { AR_CORE_CHECK_INTERNAL(__VA_ARGS__); AR_DEBUG_BREAK(); } }
    #define AR_CHECK(check, ...)           { if(!(check)) { AR_CHECK_INTERNAL(__VA_ARGS__); AR_DEBUG_BREAK(); } }

#else

    #define AR_CORE_CHECK(check, ...)
    #define AR_CHECK(check, ...)

#endif