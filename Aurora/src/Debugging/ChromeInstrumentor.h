#pragma once

// !!-------DEPRECATED---------!!

/*
 * This is now a deprecated Profiler, it was used mainly before introducing Optick into the engine and all the profiled information were
 * dumped into a json file which then was displayed in Chrome://tracing. It was good as DIY profiling however it is nothing compared
 * to Optick which is why it is now deprecated.
 */

#include "Core/Base.h"
#include "Core/Log.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <string>
#include <thread>
#include <mutex>
#include <sstream>

namespace Aurora {

    using FloatingPointMicroseconds = std::chrono::duration<float, std::micro>;

    struct ProfileResult
    {
        std::string Name;

        FloatingPointMicroseconds Start;
        std::chrono::microseconds ElapsedTime;
        std::thread::id ThreadID;
    };

    struct InstrumentationSession
    {
        const char* Name;
    };

    class Instrumentor
    {
    public:
        Instrumentor(const Instrumentor&) = delete;
        Instrumentor(Instrumentor&&) = delete;

        void BeginSession(const char* name, const char* filepath = "results.json")
        {
            std::lock_guard lock(m_Mutex);
            
            if (!std::filesystem::exists(filepath))
                std::filesystem::create_directory(filepath);

            m_OutputStream.open(filepath);
            if (m_OutputStream.is_open())
            {
                m_CurrentSession = new InstrumentationSession{ name };
                WriteHeader();
            }
            else
            {
                if(Logger::Log::GetCoreLogger())
                    AR_CORE_ERROR_TAG("Instrumentor", "Chrome Instrumentor could not open file: {0}", filepath);
            }
        }

        void EndSession()
        {
            std::lock_guard lock(m_Mutex);
            if (m_CurrentSession)
            {
                WriteFooter();
                m_OutputStream.close();
                delete m_CurrentSession;
                m_CurrentSession = nullptr;
            }
        }

        void WriteProfile(const ProfileResult& result)
        {
            std::stringstream fileOutput;

            fileOutput << std::setprecision(3) << std::fixed;
            fileOutput << ",{";
            fileOutput << "\"cat\":\"function\",";
            fileOutput << "\"dur\":" << (result.ElapsedTime.count()) << ',';
            fileOutput << "\"name\":\"" << result.Name << "\",";
            fileOutput << "\"ph\":\"X\",";
            fileOutput << "\"pid\":0,";
            fileOutput << "\"tid\":" << result.ThreadID << ",";
            fileOutput << "\"ts\":" << result.Start.count();
            fileOutput << "}";

            std::lock_guard lock(m_Mutex);
            if (m_CurrentSession)
            {
                m_OutputStream << fileOutput.str();
                m_OutputStream.flush();
            }
        }

        static Instrumentor& Get()
        {
            static Instrumentor s_Instance;
            return s_Instance;
        }

    private:
        Instrumentor()
            : m_CurrentSession(nullptr)
        {
        }

        void WriteHeader()
        {
            m_OutputStream << "{\"otherData\": {},\"traceEvents\":[{}";
            m_OutputStream.flush();
        }

        void WriteFooter()
        {
            m_OutputStream << "]}";
            m_OutputStream.flush();
        }

    private:
        std::mutex m_Mutex;
        InstrumentationSession* m_CurrentSession;
        std::ofstream m_OutputStream;

    };

    class InstrumentationTimer
    {
    private:
        using HighResClock = std::chrono::high_resolution_clock;
        using MicroSeconds = std::chrono::microseconds;

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
            HighResClock::time_point endTimepoint = HighResClock::now();

            auto highResStart = FloatingPointMicroseconds{ m_Start.time_since_epoch() };
            auto elapsedTime = std::chrono::time_point_cast<MicroSeconds>(endTimepoint).time_since_epoch() - std::chrono::time_point_cast<MicroSeconds>(m_Start).time_since_epoch();

            Instrumentor::Get().WriteProfile({ m_Name, highResStart, elapsedTime, std::this_thread::get_id() });

            m_Stopped = true;
        }

    private:
        const char* m_Name;
        std::chrono::time_point<HighResClock> m_Start;
        bool m_Stopped;

    };

}

#ifdef AURORA_CORE_PROFILE

    #define CH_EXTENSION ".json"

    #define AR_CT_PROF_BEGIN_SESSION(name, filepath)       ::Aurora::Instrumentor::Get().BeginSession(name, filepath"/Chrome/" name CH_EXTENSION)
    #define AR_CT_PROF_END_SESSION()                       ::Aurora::Instrumentor::Get().EndSession()
    #define AR_CT_PROF_SCOPE(name)                         ::Aurora::InstrumentationTimer Instrumentor##__LINE__(name)
    #define AR_CT_PROF_FUNCTION()                          AR_CT_PROF_SCOPE(__FUNCSIG__)

#else

    #define AR_CT_PROF_BEGIN_SESSION(name, filpath)
    #define AR_CT_PROF_END_SESSION()
    #define AR_CT_PROF_SCOPE(name)
    #define AR_CT_PROF_FUNCTION()

#endif // AURORA_CORE_PROFILE