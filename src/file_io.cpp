
#include <fstream>

#include "file_io.h"
#include "utils/utils.h"

// static decls
std::string FileIO::s_lastReadFile = "";
std::string FileIO::s_lastWrittenFile = "";

//---------------------------------------------------------------------------------------
int FileIO::readFileIntoBuffer(const char *_filename, LineBuffer *_buffer)
{
    _buffer->clear();

    std::ifstream file;
    file.open(_filename, std::ios::in);

    //
    for (std::string line; std::getline(file, line); )
        _buffer->push_back(create_line((char*)line.c_str(), line.length()));

    file.close();
    s_lastReadFile = std::string(_filename);

    //
    return RETURN_SUCCESS;

}




