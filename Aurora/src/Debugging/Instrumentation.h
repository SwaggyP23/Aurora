#pragma once

#include "OptickInstrumentor.h"

#ifdef AURORA_CORE_PROFILE

    #define AR_PROFILE_BEGIN_SESSION(name, filepath)      AR_OP_START_CAPTURE()
    #define AR_PROFILE_END_SESSION(name)                  AR_OP_STOP_CAPTURE(name)
    #define AR_PROFILE_FRAME(...)                         AR_OP_PROF_FRAME(__VA_ARGS__)
    #define AR_PROFILE_FUNCTION(...)                      AR_OP_PROF_FUNCTION(__VA_ARGS__)
    #define AR_PROFILE_TAG(name, ...)                     AR_OP_PROF_TAG(name, __VA_ARGS__)
    #define AR_PROFILE_SCOPE(name)                        AR_OP_PROF_SCOPE_DYNAMIC(name)
    #define AR_PROFILE_THREAD(...)                        AR_OP_PROF_THREAD(__VA_ARGS__)

#else

    #define AR_PROFILE_BEGIN_SESSION(name, filepath)
    #define AR_PROFILE_END_SESSION(name)
    #define AR_PROFILE_FRAME(...)
    #define AR_PROFILE_FUNCTION(...)
    #define AR_PROFILE_TAG(name, ...)
    #define AR_PROFILE_SCOPE(name)
    #define AR_PROFILE_THREAD(...)

#endif