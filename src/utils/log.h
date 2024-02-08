#ifndef __LOG_H
#define __LOG_H

#include <stdio.h>

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


#endif // __LOG_H
