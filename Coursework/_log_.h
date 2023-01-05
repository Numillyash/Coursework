#ifndef _LOG__H
#define _LOG__H

#include "config.h"
#include "help.h"

void _log_start();

void _log(char *message);

void _log_ull(unsigned long long value);

#endif // !_LOG__H