#include "utils.hpp"
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>

namespace utils
{
FileLogger gLogger;
} // namespace utils

namespace jfm_debug
{
#ifdef JFM_DEBUG
char *get_address(char *str)
{
    char *start = strstr(str, "+")+1;
    JFM_ASSERT(start);
    char *end = strstr(str, ")")-1;
    JFM_ASSERT(end);
    static char addr[300];
    size_t size = end-start + 1 + 1;
    memcpy(addr, start, size);
    addr[size-1] = 0;
    return (char *)&addr[0];
}

void addr2name(const char *addr)
{
    pid_t pid = fork();

    if (pid == 0) {
        execl("/usr/bin/addr2line", "addr2line", "-f", "-C", "-e", "build/JFM", addr, NULL);
        exit(0);
    } else {
        int status;

        waitpid(pid, &status, WEXITED);
    }
}

void show_backtrace()
{
    static constexpr const size_t stack_size = 100;
    void *buffer[stack_size];
    int count;
    char **strings;

    count = backtrace(buffer, stack_size);
    JFM_ASSERT(count > 0);

    strings = backtrace_symbols(buffer, count);

    Info() << "[Dump][Start]\n";

    for (int ndx = 0; ndx < count; ++ndx) {
        Info() << "strings[" << ndx << "] : " << strings[ndx]
               << " , address : " << get_address(strings[ndx]) << "\n";
        addr2name(get_address(strings[ndx]));
    }

    Info() << "[Dump][End]\n";

    free(strings);
}
#else
void show_backtrace() {  }
#endif
} // namespac jfm_debug
