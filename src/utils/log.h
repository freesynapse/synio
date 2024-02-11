#ifndef __LOG_H
#define __LOG_H

#include <stdio.h>
#include <string.h>


//
#ifdef DEBUG
    #define LOG_INFO(...) { Log::log_("[INFO] ", __func__, __VA_ARGS__); }
    #define LOG_WARNING(...) { Log::log_("[WARN] ", __func__, __VA_ARGS__); }
    #define LOG_ERROR(...) { Log::log_("[ERR]  ", __func__, __VA_ARGS__); }
    #define LOG_CRITICAL_ERROR(...) { Log::log_("[CERR] ", __func__, __VA_ARGS__); exit(-1); }
#else
    #define LOG_INFO(...)
    #define LOG_WARNING(...)
    #define LOG_ERROR(...)
    #define LOG_CRITICAL_ERROR(...)
#endif

//
class Log
{
public:
    //
    static void open(const char *_filename="log.txt")
    { 
        #ifdef DEBUG
        file_handle = fopen(_filename, "w");
        LOG_INFO("log file '%s' created.", _filename);
        #endif
    }

    //    
    static void close() 
    {
        #ifdef DEBUG
        LOG_INFO("closing log.");
        fclose(file_handle);
        #endif

    }

    //
    static void log_(const char *_prefix, const char *_func, const char *_fmt, ...)
    {
        char *p = write_meta_to_buffer_ptr_(_prefix, _func);
     
        va_list arg_list;
        va_start(arg_list, _fmt);
        int n = vsprintf(p, _fmt, arg_list);
        va_end(arg_list);
     
        sprintf(p+n, "\n");
        fprintf(file_handle, "%s", log_buffer_ptr);

    }

    //

    static char *write_meta_to_buffer_ptr_(const char* _prefix, const char *_func)
    {
        memset(log_buffer_ptr, 0, 512);
        int n;
        char *p = log_buffer_ptr;
        n = sprintf(p, "%s", _prefix);
        p += n;
        n = sprintf(p, "%s: ", _func);
        p += n;

        return p;
    }

private:
    // member variables
    static FILE *file_handle;
    static char log_buffer_ptr[512];

};


#endif // __LOG_H
