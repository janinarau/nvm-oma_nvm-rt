#ifndef SYSTEM_SERVICE_LOGGER
#define SYSTEM_SERVICE_LOGGER

#include "system/service/OutputStream.h"

#define DEBUGGING

#ifdef DEBUGGING
#define log(stream) OutputStream::instance << "[DEBUG] " << stream << endl
#else
#define log(stream)
#endif

#define log_info(stream) OutputStream::instance << "[INFO] " << stream << endl
#define log_error(stream) OutputStream::instance << "[ERROR] " << stream << endl

#endif