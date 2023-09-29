#ifndef _AUTORIZATION__H
#define _AUTORIZATION__H

#include "config.h"
#include "_file_.h"
#define PASSWORD_FILE_NAME "password.txt"
#define KEY_FILE_NAME "serial.txt"
#define CRC_code_k 0xEB31D82EUL

void check_password();

#endif