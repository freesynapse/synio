#ifndef __FILE_IO_H
#define __FILE_IO_H

#include <stdio.h>
#include <string>
#include <set>

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
class LineBuffer; // decl for functions
class FileIO
{
public:

    // file creation/deletion
    static const std::string &create_temp_file();
    static int delete_temp_files();
    static int remove_temp_file(const std::string &_filename);

    // buffer read/write
    static int read_file_to_buffer(const std::string &_filename, LineBuffer *_line_buffer);
    static int write_buffer_to_file(const std::string &_filename, LineBuffer *_line_buffer);

    // helpers
    static bool does_file_exists(const std::string &_filename);
    static bool is_file_dir(const std::string &_filename);
    static bool is_file_temp(const std::string &_filename) { return s_tempFileList.find(_filename) != s_tempFileList.end(); }
    //
    static std::string s_lastReadFile;
    static std::string s_lastWrittenFile;
    static FileType s_lastFileType;
    static std::set<std::string> s_tempFileList;

};

//
extern const char *FileType2Str(FileType _ft);


#endif // __FILE_IO_H
