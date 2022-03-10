#pragma once
#include "config.h"
#include "bit_LA.h"

void save_open_key(char* filename, number* mod, number* pubkey);

void save_secret_key(char* filename, number* seckey);

number get_prime_from_file(char* filename, int line_number, int bit_size);