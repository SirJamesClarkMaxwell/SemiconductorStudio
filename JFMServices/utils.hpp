#pragma once
#include "pch.hpp"
#include <fstream>
#include <sstream>
#include <ostream>
#include <chrono>
#include <stdio.h>

#define BIT(x) (1 << x)

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

    struct FileLogger {
    private:
        static constexpr const char *path = "jfm.log";
        static constexpr const char *levelNames[] = {
            "NONE", "ERR", "INFO"
        };

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

        void writeMessage(int level, const char *msg)
        {
            assert(m_fileObj.is_open() == true);
            m_fileObj << "[" << FileLogger::levelNames[level] << "]" << msg;
            m_fileObj.flush();
        }
    };
    extern FileLogger gLogger;

#   define JFM_NONE            0
#   define JFM_ERR             1
#   define JFM_INFO            2

    struct Logger {
    private:
        std::ostringstream m_stream;
        std::ostream m_os;
        int m_level;
        const char *m_file;
        const char *m_function;
        int m_line;

    public:
        Logger(const char *file, const char *function, int line, int logLevel)
            : m_stream{}
            , m_os(m_stream.rdbuf())
            , m_level(logLevel)
            , m_file(file)
            , m_function(function)
            , m_line(line)
        {
            m_os << m_file << ":" << m_function << ":" << m_line << ": ";
        }

        ~Logger()
        {
            if (isVerbose())
                fprintf(m_level <= JFM_ERR ? stderr : stdout, "%s", m_stream.str().c_str());

            gLogger.writeMessage(m_level, m_stream.str().c_str());
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
    };
}

#define _Log(level)      utils::Logger(__FILE__, __func__, __LINE__, JFM_##level).stream()
#define Info()          _Log(INFO)
#define Err()           _Log(ERR)

#define MEASURE_TIME(section_name, ...) \
{ \
    auto start_time = std::chrono::steady_clock::now(); \
    __VA_ARGS__ \
    auto end_time = std::chrono::steady_clock::now(); \
    Info() << "[" << section_name << "] Time elapsed: " \
           << std::chrono::duration_cast<std::chrono::milliseconds>(end_time-start_time).count() \
           << "milliseconds.\n"; \
}
