#ifndef __FILE_IO_H
#define __FILE_IO_H

#include <stdio.h>
#include <string>

#include "../buffer/line_buffer.h"

//
enum FileType
{
    C_CPP = 0,
    PY,
    JS,
    TXT,

    DEFAULT
};

//
class FileIO
{
public:
    //
    static int read_file_to_buffer(const std::string &_filename, LineBuffer *_line_buffer);
    static int write_buffer_to_file(const std::string &_filename, LineBuffer *_line_buffer);

    // helpers
    static bool file_exists(const std::string &_filename);

    //
    static std::string s_lastReadFile;
    static std::string s_lastWrittenFile;
    static FileType s_lastFileType;

};

//
extern const char *FileType2Str(FileType _ft);


#endif // __FILE_IO_H
