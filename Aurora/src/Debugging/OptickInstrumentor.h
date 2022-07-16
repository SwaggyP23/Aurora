#pragma once

#include <optick/optick.h>

#ifdef AURORA_CORE_PROFILE

    #define AR_OP_START_CAPTURE()             OPTICK_START_CAPTURE()
    #define AR_OP_STOP_CAPTURE(filepath)      OPTICK_STOP_CAPTURE();OPTICK_SAVE_CAPTURE(filepath)
    #define AR_OP_PROF_FRAME(...)             OPTICK_FRAME(__VA_ARGS__)
    #define AR_OP_PROF_FUNCTION(...)          OPTICK_EVENT(__VA_ARGS__)
    #define AR_OP_PROF_TAG(name, ...)         OPTICK_TAG(name, __VA_ARGS__)
    #define AR_OP_PROF_SCOPE_DYNAMIC(name)    OPTICK_EVENT_DYNAMIC(name)
    #define AR_OP_PROF_THREAD(...)            OPTICK_THREAD(__VA_ARGS__) // Not that this will be used anytime soon.... xD

#else

    #define AR_OP_START_CAPTURE()
    #define AR_OP_STOP_CAPTURE(filpath)
    #define AR_OP_PROF_FRAME(...)
    #define AR_OP_PROF_FUNCTION(...)
    #define AR_OP_PROF_TAG(name, ...)
    #define AR_OP_PROF_SCOPE_DYNAMIC(name)
    #define AR_OP_PROF_THREAD(...)

#endif