
#include "file_io.h"

#include <fstream>
#include <algorithm>
#include <sys/stat.h>

#include "utils.h"

// static decls
std::string FileIO::s_lastReadFile = "";
std::string FileIO::s_lastWrittenFile = "";
FileType FileIO::s_lastFileType = UNKNOWN;

//---------------------------------------------------------------------------------------
int FileIO::read_file_to_buffer(const std::string &_filename, LineBuffer *_buffer)
{
    _buffer->clear();

    // filetype -- only deduced from file extension
    size_t pos = _filename.find_last_of('.') + 1;
    std::string file_ext = _filename.substr(pos, std::string::npos);
    // convert to lower case
    std::transform(file_ext.begin(),
                   file_ext.end(),
                   file_ext.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    LOG_INFO("file_ext '%s'", file_ext.c_str());

    // ifs for now
    if (file_ext == "cpp" || file_ext == "c" || file_ext == "cxx" || file_ext == "h" || file_ext == "hpp")
        s_lastFileType = C_CPP;

    //
    std::ifstream file;
    file.open(_filename, std::ios::in);

    //
    std::string line;
    while (std::getline(file, line))
        _buffer->push_back(create_line((char *)line.c_str(), line.length()));

    file.close();
    s_lastReadFile = std::string(_filename);

    //
    return RETURN_SUCCESS;

}

//---------------------------------------------------------------------------------------
int FileIO::write_buffer_to_file(const std::string &_filename, LineBuffer *_line_buffer)
{
    std::ofstream file;
    file.open(_filename, std::ios::out);

    std::string line;
    line_t *buffer_line_ptr = _line_buffer->m_head;
    size_t nbytes = 0;
    while (buffer_line_ptr != NULL)
    {
        file << buffer_line_ptr->__debug_str << '\n';
        nbytes += buffer_line_ptr->len + 1;
        buffer_line_ptr = buffer_line_ptr->next;
    }

    LOG_INFO("wrote %zu byte(s) to '%s'", nbytes, _filename.c_str());
    s_lastWrittenFile = _filename;

    file.close();
    //
    return RETURN_SUCCESS;

}

//---------------------------------------------------------------------------------------
bool FileIO::file_exists(const std::string &_filename)
{
    struct stat buffer;
    return stat(_filename.c_str(), &buffer) == 0 ? true : false;

}

