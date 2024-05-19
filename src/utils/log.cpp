
#include "log.h"

// static decls
FILE *Log::s_file_handle = NULL;
char Log::s_log_buffer_ptr[512];
std::string Log::s_filename = "";

