
#include "log.h"

// static decls
FILE *Log::file_handle = NULL;
char Log::log_buffer_ptr[512];
std::string Log::filename = "";
