#ifndef __LOG_H
#define __LOG_H

#include <stdio.h>
#include <string.h>
#include <string>
#include <assert.h>

//
#define __func__ __PRETTY_FUNCTION__

#ifdef DEBUG
    #define LOG_RAW(...) { Log::open(); Log::log_("", "", __VA_ARGS__); fclose(Log::s_file_handle); }
    #define LOG_INFO(...) { Log::open(); Log::log_("\x1b[32m[INFO]\x1b[0m ", __func__, __VA_ARGS__); fclose(Log::s_file_handle); }
    #define LOG_WARNING(...) { Log::open(); Log::log_("\x1b[33m[WARN]\x1b[0m ", __func__, __VA_ARGS__); fclose(Log::s_file_handle); }
    #define LOG_ERROR(...) { Log::open(); Log::log_("\x1b[31m[ERR]\x1b[0m  ", __func__, __VA_ARGS__); fclose(Log::s_file_handle); }
    #define LOG_CRITICAL_ERROR(...) { Log::open(); Log::log_("\x1b[31m[CERR]\x1b[0m ", __func__, __VA_ARGS__); fclose(Log::s_file_handle); int *a = NULL; *a = 1; }
    #define NOT_IMPLEMENTED() { LOG_ERROR("not yet implemented"); }
#else
    #define LOG_RAW(...)
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
    static void open(const char *_filename="")
    {
        if (s_filename == "")
        {
            s_filename = std::string(_filename);
            s_file_handle = fopen(_filename, "w");
            log_("\x1b[32m[INFO]\x1b[0m ", __func__, "log file '%s' created.", _filename);
            fclose(s_file_handle);
        }
        else
            s_file_handle = fopen(s_filename.c_str(), "a");
    }

    //    
    static void close() 
    {
        LOG_INFO("closing log.");
        //printf("file handle %p\n", file_handle);
        //if (file_handle)
        //    fclose(file_handle);

    }

    //
    static void log_(const char *_prefix, const char *_func, const char *_fmt, ...)
    {
        char *p = write_meta_to_buffer_ptr_(_prefix, _func);
        if (p - s_log_buffer_ptr != 0)
        {
            sprintf(p, ": ");
            p += 2;
        }

        va_list arg_list;
        va_start(arg_list, _fmt);
        int n = vsprintf(p, _fmt, arg_list);
        va_end(arg_list);
     
        sprintf(p+n, "\n");
        fprintf(s_file_handle, "%s", s_log_buffer_ptr);

    }

    //

    static char *write_meta_to_buffer_ptr_(const char* _prefix, const char *_func)
    {
        memset(s_log_buffer_ptr, 0, 512);
        int n;
        char *p = s_log_buffer_ptr;
        n = sprintf(p, "%s", _prefix);
        p += n;
        n = sprintf(p, "%s", _func);
        p += n;

        return p;
    }

// private:
    // member variables
    static FILE *s_file_handle;
    static char s_log_buffer_ptr[512];
    static std::string s_filename;

};


#endif // __LOG_H
