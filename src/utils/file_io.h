#ifndef __FILE_IO_H
#define __FILE_IO_H

#include <stdio.h>
#include <string>

#include "../buffer/line_buffer.h"

//
class FileIO
{
public:
    static int readFileIntoBuffer(const std::string &_filename, LineBuffer *_line_buffer);

    // accessors
    const char *lastReadFile() { return s_lastReadFile.c_str(); }
    const char *lastWrittenFile() { return s_lastWrittenFile.c_str(); }
    

private:
    static std::string s_lastReadFile;
    static std::string s_lastWrittenFile;

};



#endif // __FILE_IO_H
