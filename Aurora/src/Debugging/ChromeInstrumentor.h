#pragma once

/*
 * This is now a deprecated Profiler, it was used mainly before introducing Optick into the engine and all the profiled information were
 * dumped into a json file which then was displayed in Chrome://tracing. It was good as DIY profiling however it is nothing compared
 * to Optick which is why it is now deprecated.
 */

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <string>
#include <thread>
#include <mutex>
#include <sstream>

namespace Aurora {

    struct ProfileResult
    {
        std::string Name;
        long long Start, End;
        // uint32_t ThreadID;
    };

    struct InstrumentationSession
    {
        const char* Name;
    };

    class Instrumentor
    {
    public:
        Instrumentor()
            : m_CurrentSession(nullptr), m_ProfileCount(0)
        {
        }

        void BeginSession(const char* name, const char* filepath = "results.json")
        {
            m_OutputStream.open(filepath);
            WriteHeader();
            m_CurrentSession = new InstrumentationSession{ name };
        }

        void EndSession()
        {
            WriteFooter();
            m_OutputStream.close();
            delete m_CurrentSession;
            m_CurrentSession = nullptr;
            m_ProfileCount = 0;
        }

        void WriteProfile(const ProfileResult& result)
        {
            if (m_ProfileCount++ > 0)
                m_OutputStream << ",";

            std::string name = result.Name;
            std::replace(name.begin(), name.end(), '"', '\'');

            m_OutputStream << "{";
            m_OutputStream << "\"cat\":\"function\",";
            m_OutputStream << "\"dur\":" << (result.End - result.Start) << ',';
            m_OutputStream << "\"name\":\"" << name << "\",";
            m_OutputStream << "\"ph\":\"X\",";
            m_OutputStream << "\"pid\":0,";
            m_OutputStream << "\"tid\":0,";
            m_OutputStream << "\"ts\":" << result.Start;
            m_OutputStream << "}";

            m_OutputStream.flush();
        }

        void WriteHeader()
        {
            m_OutputStream << "{\"otherData\": {},\"traceEvents\":[";
            m_OutputStream.flush();
        }

        void WriteFooter()
        {
            m_OutputStream << "]}";
            m_OutputStream.flush();
        }

        static Instrumentor& Get()
        {
            static Instrumentor instance;
            return instance;
        }

    private:
        InstrumentationSession* m_CurrentSession;
        std::ofstream m_OutputStream;
        int m_ProfileCount;

    };

    class InstrumentationTimer
    {
    public:
        InstrumentationTimer(const char* name)
            : m_Name(name), m_Stopped(false)
        {
            m_Start = std::chrono::high_resolution_clock::now();
        }

        ~InstrumentationTimer()
        {
            if (!m_Stopped)
                Stop();
        }

        void Stop()
        {
            auto endTimepoint = std::chrono::high_resolution_clock::now();

            long long start = (long long)(std::chrono::time_point_cast<std::chrono::nanoseconds>(m_Start).time_since_epoch().count() * 0.001);
            long long end = (long long)(std::chrono::time_point_cast<std::chrono::nanoseconds>(endTimepoint).time_since_epoch().count() * 0.001);

            // uint32_t threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
            Instrumentor::Get().WriteProfile({ m_Name, start, end });

            m_Stopped = true;
        }

    private:
        const char* m_Name;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_Start;
        bool m_Stopped;

    };

}

#ifdef AURORA_CORE_PROFILE

    #define AR_CT_PROF_BEGIN_SESSION(name, filpath)        ::Aurora::Instrumentor::Get().BeginSession(name, filpath)
    #define AR_CT_PROF_END_SESSION()                       ::Aurora::Instrumentor::Get().EndSession()
    #define AR_CT_PROF_SCOPE(name)                         ::Aurora::InstrumentationTimer Instrumentor##__LINE__(name)
    #define AR_CT_PROF_FUNCTION()                          AR_CT_PROF_SCOPE(__FUNCSIG__)

#else

    #define AR_CT_PROF_BEGIN_SESSION(name, filpath)
    #define AR_CT_PROF_END_SESSION()
    #define AR_CT_PROF_SCOPE(name)
    #define AR_CT_PROF_FUNCTION()

#endif