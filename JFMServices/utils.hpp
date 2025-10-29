#pragma once
#include "pch.hpp"
#include <fstream>
#include <sstream>
#include <ostream>
#include <format>
#include <chrono>
#include <stdio.h>

#define BIT(x) (1 << x)
#define ARRAY_LENGTH(arr)       ((size_t)(sizeof(arr)/sizeof(arr[0])))

namespace utils
{

    inline void generateVectorAtGivenRanges(std::vector<double> &destination, double min, double max, double step)
    {
        destination.clear();
        int size = static_cast<int>((max - min) / step) + 1;
        destination.resize(size);

        int count = -1;
        std::ranges::generate(destination.begin(), destination.end(), [&]()
                              { count++; return min + count * step; });
    };

    inline std::vector<std::string> spliting(const std::string &str, const char *delimiter)
    {
        std::vector<std::string> tokens;
        std::string::size_type start = 0;
        std::string::size_type end = 0;

        while ((end = str.find(delimiter, start)) != std::string::npos)
        {
            tokens.push_back(str.substr(start, end - start));
            start = end + 1;
        }
        tokens.push_back(str.substr(start));

        return tokens;
    };
    template <size_t size>
    inline std::array<double, size> cast(const std::valarray<double> &source)
    {
        std::array<double, size> destination;
        for (const auto &[dest, src] : std::views::zip(destination, source))
            dest = src;
        return destination;
    }

    static const char *jfmLevelNames[] = {
        "NONE", "ERR", "INFO", "VERBOSE", "TRACE"
    };
    enum JfmLogLevel : uint8_t {
        None = 0,
        Err,
        Info,
        Verbose,
        Trace,
        Count
    };
    static_assert(JfmLogLevel::Count == ARRAY_LENGTH(jfmLevelNames));

    struct FileLogger {
    private:
        static constexpr const char *path = "jfm.log";

        std::ofstream m_fileObj;

    public:
        FileLogger()
        {
            m_fileObj = std::ofstream(FileLogger::path, std::ios_base::app);
            assert(m_fileObj.is_open() == true);
        }

        ~FileLogger()
        {
            m_fileObj.close();
        }

        void writeMessage(const char *msg)
        {
            assert(m_fileObj.is_open() == true);
            m_fileObj << msg;
            m_fileObj.flush();
        }
    };
    extern FileLogger gLogger;

    inline static JfmLogLevel getLogLevel()
    {
        const char *level = getenv("JFM_LOG_LEVEL");
        if (level == NULL)
            return JfmLogLevel::None;

        int value = atoi(level);

        switch (value)
        {
        case 1: return JfmLogLevel::Err;
        case 2: return JfmLogLevel::Info;
        case 3: return JfmLogLevel::Trace;
        case 0:
        default:
            break;
        }

        return JfmLogLevel::None;
    }

    static const JfmLogLevel gLogLevel = getLogLevel();


    struct Logger {
    private:
        std::ostringstream m_stream;
        std::ostream m_os;
        JfmLogLevel m_level;
        const char *m_file;
        const char *m_function;
        int m_line;

    public:
        Logger(const char *file, const char *function, int line, enum JfmLogLevel logLevel)
            : m_stream{}
            , m_os(m_stream.rdbuf())
            , m_level(logLevel)
            , m_file(file)
            , m_function(function)
            , m_line(line)
        {
            static char header[17]; // Note: we are using fixed number of characters: 4 + 5 + 6 + 1 + 1
            // our header is:
            //         '[' + jfmLevelNames padded with ' '         + ']'
            //         '[' + first 6 digits of calling thread's id + ']' + ' '
            snprintf(header, sizeof header, "[%5.5s][%6.6s] ",
                jfmLevelNames[m_level],
                std::to_string( std::hash<std::thread::id>{}(std::this_thread::get_id()) ).c_str() );

            m_os << header << m_file << ":" << m_function << ":" << m_line << ": ";
        }

        ~Logger()
        {
            if (isLoggerOn(m_level))
            {
                if (isVerbose())
                    fprintf(m_level <= JfmLogLevel::Err ? stderr : stdout, "%s", m_stream.str().c_str());

                gLogger.writeMessage(m_stream.str().c_str());
            }
        }

        std::ostream &stream() { return m_os; }

    private:
        inline bool isVerbose()
        {
            static bool verbose = false;
            {
                static bool setupDone = false;
                if (setupDone == false)
                {
#if defined(JFM_PLATFORM_NIX)
                    const char *jfmVerbose = NULL;

                    if ((jfmVerbose = getenv("JFM_VERBOSE")) && (atoi(jfmVerbose) == 1))
                        verbose = true;
#else
                        verbose = true;
#endif
                    setupDone = true;
                }
            }
            return verbose;
        }

        inline static bool isLoggerOn(JfmLogLevel level)
        {
            return gLogLevel >= level;
        }
    };

    template <typename T>
    inline T min(const T& a, const T&b)
    {
        return a > b ? b : a;
    }
}

#define _Log(level)      utils::Logger(__FILE__, __func__, __LINE__, utils::JfmLogLevel::level).stream()
#define Err()           _Log(Err)
#define Info()          _Log(Info)
#define Verbose()       _Log(Verbose)
#define Trace()         _Log(Trace) << "\n"

#define _MEASURE_TIME(logger, precision, section_name, ...) \
do { \
    auto start_time = std::chrono::steady_clock::now(); \
    __VA_ARGS__ \
    auto end_time = std::chrono::steady_clock::now(); \
    logger() << "[" << section_name << "] Time elapsed: " \
           << std::chrono::duration_cast<std::chrono::precision>(end_time-start_time).count() \
           << #precision << ".\n"; \
} while (0)

#define MEASURE_TIME(...)                   _MEASURE_TIME(Info, milliseconds, __VA_ARGS__)
#define MEASURE_TIME_PRECISE(...)           _MEASURE_TIME(Trace, microseconds, __VA_ARGS__)
#define MEASURE_TIME_THIS_FUNC(...)         MEASURE_TIME(__func__, __VA_ARGS__)

#define Unreachable()           assert("Unreachable reached !" == 0)
