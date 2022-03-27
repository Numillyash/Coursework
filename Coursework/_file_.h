#pragma once
#include "config.h"
#include "bit_LA.h"

/// <summary>
/// Проверка на существование файла (запись)
/// </summary>
/// <param name="filename">Имя файла в виде строки</param>
/// <returns>Указатель на файл</returns>
FILE* check_file_exist_write(char* filename);

/// <summary>
/// Проверка на существование файла (чтение)
/// </summary>
/// <param name="filename">Имя файла в виде строки</param>
/// <returns>Указатель на файл</returns>
FILE* check_file_exist_read(char* filename);

void save_open_key(char* filename, number* mod, number* pubkey);

void save_secret_key(char* filename, number* seckey);

/// <summary>
/// Читает открытый ключ из файла
/// </summary>
/// <param name="filename">имя файла</param>
/// <param name="mod">Указатель на неинициализированное число</param>
/// <param name="pubkey">Указатель на неинициализированное число</param>
void read_open_key(char* filename, number* mod, number* pubkey);

void read_secret_key(char* filename, number* seckey);

number get_prime_from_file(char* filename, int line_number, int bit_size);