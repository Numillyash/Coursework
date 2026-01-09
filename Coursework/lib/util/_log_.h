#ifndef _LOG__H
#define _LOG__H

#include "../../include/config.h"
#include "../../include/help.h"

#ifdef NO_LOG
static inline void _log_start(void) { (void)0; }
static inline void _log(char *message) { (void)message; }
static inline void _log_ull(unsigned long long value) { (void)value; }
#else
void _log_start();
void _log(char *message);
void _log_ull(unsigned long long value);
#endif

#endif // !_LOG__H