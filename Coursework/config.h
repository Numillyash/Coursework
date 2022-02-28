#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define DEBUG

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <malloc.h>
#include <time.h>
#include <stdint.h>

//input console checking
#define HELP_CONSOLE_OPTION_1 "-h"
#define HELP_CONSOLE_OPTION_2 "--help"
#define GENKEY_CONSOLE_OPTION "genkey"
#define SIGNATURE_CONSOLE_OPTION "sign"
#define CHECK_CONSOLE_OPTION "check"
#define ENCRYPT_CONSOLE_OPTION "encrypt"
#define DECRYPT_CONSOLE_OPTION "decrypt"
#define SIZE_CONSOLE_OPTION "--size"
#define PUBKEY_CONSOLE_OPTION "--pubkey"
#define SECRET_CONSOLE_OPTION "--secret"
#define SIGFILE_CONSOLE_OPTION "--sigfile"
#define INFILE_CONSOLE_OPTION "--infile"
#define OUTFILE_CONSOLE_OPTION "--outfile"

//RSA
#define KEYSIZE_MODULE 256
typedef enum {
	KEY_256 = 8,
	KEY_512,
	KEY_1024,
	KEY_2048
} KEY_BIT_SIZE;

//file
#define FILE_OPENING_ERROR "\
Cannot open the %s file.\n"

/// <summary>
/// Exit codes for exit()
/// </summary>
enum EXIT_CODE {
	SUCCESS,
	FAILURE,
	NO_ARGUMENTS_FAILURE,
	WRONG_ARGUMENT_FAILURE,
	NOT_ENOUGH_ARGUMENTS_FAILURE,
	FILE_OPEN_ERROR_FAILURE,
	MEMORY_ALLOCATION_FAILURE
};

//fast funcs
#define swap(a,b) b = a+b; a = b-a; b = b-a;