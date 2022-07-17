#pragma once

#include "ChromeInstrumentor.h"
#include "OptickInstrumentor.h"

// If you want to use Optick for profiling leave it at 1, however if for some reason you want to use Chrome Tracing (Not advised) make it 0
#define AURORA_USE_OPTICK 1

#ifdef AURORA_CORE_PROFILE

    #if AURORA_USE_OPTICK

        #define AR_PROFILE_BEGIN_SESSION(name, filepath)      AR_OP_START_CAPTURE()
        #define AR_PROFILE_END_SESSION(name)                  AR_OP_STOP_CAPTURE(name)
        #define AR_PROFILE_FRAME(...)                         AR_OP_PROF_FRAME(__VA_ARGS__)
        #define AR_PROFILE_FUNCTION(...)                      AR_OP_PROF_FUNCTION(__VA_ARGS__)
        #define AR_PROFILE_TAG(name, ...)                     AR_OP_PROF_TAG(name, __VA_ARGS__)
        #define AR_PROFILE_SCOPE(name)                        AR_OP_PROF_SCOPE_DYNAMIC(name)
        #define AR_PROFILE_THREAD(...)                        AR_OP_PROF_THREAD(__VA_ARGS__)

    #else

        #define AR_PROFILE_BEGIN_SESSION(name, filepath)      AR_CT_PROF_BEGIN_SESSION(name, filepath)
        #define AR_PROFILE_END_SESSION(name)                  AR_CT_PROF_END_SESSION()
        #define AR_PROFILE_FRAME(...)
        #define AR_PROFILE_FUNCTION(...)                      AR_CT_PROF_FUNCTION()
        #define AR_PROFILE_TAG(name, ...)
        #define AR_PROFILE_SCOPE(name)                        AR_CT_PROF_SCOPE(name)
        #define AR_PROFILE_THREAD(...)

    #endif // AURORA_USE_OPTICK

#else

    #define AR_PROFILE_BEGIN_SESSION(name, filepath)
    #define AR_PROFILE_END_SESSION(name)
    #define AR_PROFILE_FRAME(...)
    #define AR_PROFILE_FUNCTION(...)
    #define AR_PROFILE_TAG(name, ...)
    #define AR_PROFILE_SCOPE(name)
    #define AR_PROFILE_THREAD(...)

#endif