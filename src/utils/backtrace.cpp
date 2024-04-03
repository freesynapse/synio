
#include <dlfcn.h>
#include <stdint.h>

#include "backtrace.h"
#include "log.h"
#include "../core.h"

#define BACKTRACE_LOG(...) { fprintf(stderr, __VA_ARGS__); fprintf(Log::file_handle, __VA_ARGS__); }

void signal_handler(int _sig_num, siginfo_t *_info, void *_ucontext)
{
    LOG_ERROR("SIGSEGV recieved (%s)", strsignal(_sig_num));

    //
    api->shutdown();

    sig_ucontext_t *uc = (sig_ucontext_t *)_ucontext;
    void *caller_addr;
    #if defined(__i386__)
        caller_addr = (void *)uc->uc_mcontext.eip;
    #elif defined(__x86_64__)
        caller_addr = (void *)uc->uc_mcontext.rip;
    #endif

    #define BACKTRACE_ADDR_COUNT 64
    void *backtraceAddrs[BACKTRACE_ADDR_COUNT];
    int addrCount;
    char **backtraceMsgs;

    // output stack bactrace
    addrCount = backtrace(backtraceAddrs, BACKTRACE_ADDR_COUNT);
    backtraceAddrs[1] = caller_addr;
    // to output function names (from the dynamic symbols table) compiler flag -rdynamic
    // is required for clang (and gcc, apparantly)
    backtraceMsgs = backtrace_symbols(backtraceAddrs, addrCount);
    BACKTRACE_LOG("\x1b[1;37;41m--------------------- STACKTRACE ---------------------\x1b[0m\n");
    BACKTRACE_LOG("signal %d (%s) address is 0x%lx from 0x%lx.\n", \
                  _sig_num, strsignal(_sig_num), (uintptr_t)_info->si_addr, (uintptr_t)caller_addr);
    for (int i = 1; i < addrCount && backtraceMsgs != NULL; i++)
        BACKTRACE_LOG("%3d : %s\n", i, backtraceMsgs[i]);
    BACKTRACE_LOG("\x1b[1;37;41m------------------- END STACKTRACE -------------------\x1b[0m\n");

    free(backtraceMsgs);
    // fclose(f_restore);
    Log::close();
    
    // restore and trigger default signal handler (triggering a core dump)
    signal(_sig_num, SIG_DFL);
    kill(getpid(), _sig_num);
    
}

