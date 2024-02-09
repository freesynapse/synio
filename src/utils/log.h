#ifndef __LOG_H
#define __LOG_H

#include <stdio.h>
#include <string.h>

//
#ifdef DEBUG
    #define LOG_INFO(...) fprintf(Log::file_handle, "[INFO] " __VA_ARGS__)
    #define LOG_WARNING(...) fprintf(Log::file_handle, "[WARNING] " __VA_ARGS__)
    #define LOG_ERROR(...) { fprintf(Log::file_handle, "[ERROR] " __VA_ARGS__); exit(-1); }
#else
    #define LOG_INFO(...)
    #define LOG_WARNING(...)
    #define LOG_ERROR(...)
#endif

//
class Log
{
public:
    //
    static void open(const char *_filename="log.txt")
    { 
        file_handle = fopen(_filename, "w");
        LOG_INFO("%s: log file '%s' created.\n", __func__, _filename);
        
    }
    
    static void close() 
    {
        LOG_INFO("%s: closing log.\n", __func__);
        fclose(file_handle);

    }

    // member variables
    static FILE *file_handle;

};

// TODO : finish this! 
// GOAL : append a '\n' to the string automatically. Nearly there, but no time...
static char LOG_BUFFER[1024];
inline void LOG_(const char *_prefix, const char *_fmt, ...)
{
    int n = 0;
    memset(LOG_BUFFER, 0, 1024);
    char *p = LOG_BUFFER;
    n = sprintf(p, "%s", _prefix);
    p += n;

    va_list arg_list;
    
    va_start(arg_list, _fmt);
    n = vsprintf(LOG_BUFFER, _fmt, arg_list);
    va_end(arg_list);

    sprintf(LOG_BUFFER+n, "\n");

}



#endif // __LOG_H
