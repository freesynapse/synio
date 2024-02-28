
#include <fstream>

#include "file_io.h"
#include "utils.h"

// static decls
std::string FileIO::s_lastReadFile = "";
std::string FileIO::s_lastWrittenFile = "";

//---------------------------------------------------------------------------------------
int FileIO::readFileIntoBuffer(const std::string &_filename, MultiLineBuffer *_buffer)
{
    _buffer->clear();

    std::ifstream file;
    file.open(_filename, std::ios::in);

    //
    std::string line;
    while (std::getline(file, line))
        _buffer->push_back(create_line((char *)line.c_str(), line.length()));

    // if (_buffer->m_tail->content[_buffer->m_tail->len] != '\n')
    // {
    //     LOG_WARNING("no EOF newline in %s.", _filename.c_str());
    //     _buffer->push_back(create_line(""));
    // }

    file.close();
    s_lastReadFile = std::string(_filename);

    //
    return RETURN_SUCCESS;

}




