#pragma once

// This is to be used with Chrome Tracing

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

#ifdef AURORA_CORE_PROFILE_

    #define AR_PROFILE_BEGIN_SESSION(name, filpath)        ::Aurora::Instrumentor::Get().BeginSession(name, filpath)
    #define AR_PROFILE_END_SESSION()                       ::Aurora::Instrumentor::Get().EndSession()
    #define AR_PROFILE_SCOPE(name)                         ::Aurora::InstrumentationTimer timer##__LINE__(name)
    #define AR_PROFILE_FUNCTION()                          AR_PROFILE_SCOPE(__FUNCSIG__)

#else

    #define AR_PROFILE_BEGIN_SESSION(name, filpath)
    #define AR_PROFILE_END_SESSION()
    #define AR_PROFILE_SCOPE(name)
    #define AR_PROFILE_FUNCTION()

#endif