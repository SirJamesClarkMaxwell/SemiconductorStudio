#pragma once

#include <assert.h>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <thread>
#include <sstream>
#include <time.h>
#include <string.h>

#define BIT(x) (1 << x)
#define ARRAY_LENGTH(arr)       ((size_t)(sizeof(arr)/sizeof(arr[0])))

#define _Log(level)     utils::Logger(__FILE__, __func__, __LINE__, utils::JfmLogLevel::level).stream()
#define Log()           utils::Logger(utils::JfmLogLevel::Info).stream()
#define Err()           _Log(Err)
#define Info()          _Log(Info)
#define Verbose()       _Log(Verbose)
#define Trace()         _Log(Trace) << "\n"

#define _MEASURE_TIME(logger, precision, section_name, ...)                                     \
do {                                                                                            \
    Info() << "[Measurement][Start]\n";                                                         \
    auto start_time = std::chrono::steady_clock::now();                                         \
    __VA_ARGS__                                                                                 \
    auto end_time = std::chrono::steady_clock::now();                                           \
    Info() << "[Measurement][End]\n";                                                           \
    logger() << "[" << section_name << "] Time elapsed: "                                       \
           << std::chrono::duration_cast<std::chrono::precision>(end_time-start_time).count()   \
           << " " #precision << ".\n";                                                          \
} while (0)

#define MEASURE_TIME(...)                   _MEASURE_TIME(Info, milliseconds, __VA_ARGS__)
#define MEASURE_TIME_PRECISE(...)           _MEASURE_TIME(Trace, microseconds, __VA_ARGS__)
#define MEASURE_TIME_THIS_FUNC(...)         MEASURE_TIME(__func__, __VA_ARGS__)

#define Unreachable()                       JFM_ASSERT("Unreachable reached !" == 0)

#ifdef JFM_DEBUG
#define JFM_ASSERT(cond)                    assert(cond)
#define JFM_ASSERT_WITH_MSG(cond, fmt, args...) do {                                            \
    if ( ! (cond)) {                                                                            \
        char errLog[1000];                                                                      \
        Err() << "Command \"" << #cond << "\" failed !\n";                                      \
        sprintf(errLog, fmt, ##args);                                                           \
        Err() << errLog;                                                                        \
        abort();                                                                                \
    }                                                                                           \
} while (0)

#else

#define JFM_ASSERT(cond)                        (cond)
#define JFM_ASSERT_WITH_MSG(cond, fmt, args...) (cond)
#endif

#define JFM_UNUSED                          [[maybe_unused]]

#if defined(_MSC_VER_)
#define JFM_INLINE                          __forceinline
#else
#define JFM_INLINE                          inline
#endif

namespace utils
{
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
            JFM_ASSERT(m_fileObj.is_open() == true);

            JFM_ASSERT(freopen(FileLogger::path, "a", stdout) != (FILE *)0);
            JFM_ASSERT(freopen(FileLogger::path, "a", stderr) != (FILE *)0);
        }

        ~FileLogger()
        {
            m_fileObj.close();
        }

        void writeMessage(const char *msg)
        {
            JFM_ASSERT(m_fileObj.is_open() == true);
            m_fileObj << msg;
            m_fileObj.flush();
        }
    };
    extern FileLogger gLogger;

    inline static JfmLogLevel getLogLevel()
    {
        JfmLogLevel ret;
        const char *level = getenv("JFM_LOG_LEVEL");
        if (level == NULL)
            return JfmLogLevel::None;

        int value = atoi(level);

        switch (value)
        {
        case 2:
            ret = JfmLogLevel::Info;
            break;
        case 3:
            ret = JfmLogLevel::Trace;
            break;
        case 0:
            ret = JfmLogLevel::None;
        default:
        case 1:
            ret = JfmLogLevel::Err;
            break;
        }

        return ret;
    }

    static const JfmLogLevel gLogLevel = getLogLevel();

    static inline bool isVerbose()
    {
        const char *jfmVerbose = NULL;
        return ((jfmVerbose = getenv("JFM_VERBOSE")) && (atoi(jfmVerbose) == 1));
    }
    static const bool gIsVerbose = isVerbose();

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

        Logger(enum JfmLogLevel logLevel)
            : m_stream{}
            , m_os(m_stream.rdbuf())
            , m_level(logLevel)
        {
        }

        ~Logger()
        {
            if (isLoggerOn(m_level))
            {
                if (gIsVerbose)
                    fprintf(m_level <= JfmLogLevel::Err ? stderr : stdout, "%s", m_stream.str().c_str());

                gLogger.writeMessage(m_stream.str().c_str());
            }
        }

        std::ostream &stream() { return m_os; }

    private:
        inline static bool isLoggerOn(JfmLogLevel level)
        {
            return gLogLevel >= level;
        }
    };

} // namespace utils

namespace jfm_debug
{
#if defined(JFM_DEBUG)
    template <typename T>
    struct DataDumper {
    private:
        static constexpr const char *folder_path = "/home/chris/tmp/random/SemiconductorStudio/dumps";
        char m_file_path[100];
        std::ofstream m_file;

        const T *m_data;
        size_t m_size;
        char *m_tag;

    void create_file_path()
    {
        struct tm *tm;
        time_t t;

        t = time(NULL);

        tm = localtime(&t);

        sprintf(m_file_path, "%s/%s-%d-%02d-%02d %02d:%02d:%02d.dump",
            DataDumper::folder_path, m_tag,
            tm->tm_year, tm->tm_mon, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec);
    }

    public:
        DataDumper(const char *tag,
                   const T *data,
                   const size_t size)
            : m_data(data)
            , m_size(size)
        {
            JFM_ASSERT(this->m_data);
            JFM_ASSERT(this->m_size);

            m_tag = new char[strlen(tag)];
            strcpy(m_tag, tag);

            create_file_path();
            m_file = std::ofstream(m_file_path, std::ios::out);
            JFM_ASSERT(m_file.is_open());
        }

        ~DataDumper()
        {
            for (size_t ndx = 0; ndx < m_size; ++ndx)
                m_file << m_data[ndx] << "\n";
            m_file.close();
            delete[] m_tag;
        }
    };
#else
    template <typename T>
    struct DataDumper
    {
    public:
        DataDumper(JFM_UNUSED const char *tag,
                   JFM_UNUSED const T *data,
                   JFM_UNUSED const size_t size)
        {
        }
    };

#endif
} // namespace jfm_debug
