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

/// <summary>
/// Запись number в файл
/// </summary>
/// <param name="file">указатель на файл</param>
/// <param name="value">число</param>
void save_num_to_file(FILE* file, number* value);

/// <summary>
/// Чтение number из файла
/// </summary>
/// <param name="file">указатель на файл</param>
/// <param name="value">число</param>
int read_num_from_file(FILE* file, number* value);

/// <summary>
/// Сохраняет ключ
/// </summary>
/// <param name="filename">имя файла</param>
/// <param name="mod">N</param>
/// <param name="subkey">второй параметр (e или d)</param>
void save_key(char* filename, number* mod, number* subkey, char log);

/// <summary>
/// Читает ключ из файла
/// </summary>
/// <param name="filename">имя файла</param>
/// <param name="mod">N</param>
/// <param name="subkey">второй параметр (e или d)</param>
void read_key(char* filename, number* mod, number* subkey, char log);

number get_prime_from_file(char* filename, int line_number, int bit_size);