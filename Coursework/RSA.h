#pragma once
#include "config.h"
#include "_file_.h"

void decrypt_file(char* input_filename, char* seckey_filename, char* output_filename);

void encrypt_file(char* input_filename, char* pubkey_filename, char* output_filename);

void generate_key(char* key_size_str, char* pubkey_filename, char* seckey_filename);

void signify_file(char* input_filename, char* sec_code_filename, char* output_filename);
