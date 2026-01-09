#ifndef _FILE__H
#define _FILE__H

#include "../bigint/bit_LA.h"

#define PREFIX_SIZE 3
#define PRIME_PREFIX_SIZE 5

/// <summary>
/// Проверяет имена файлов на правильность формата и совпадение
/// </summary>
/// <param name="fn1">Имя первого файла</param>
/// <param name="fn2">Имя второго файла</param>
/// <returns>1, если все проверки пройдены. 0, если хотя бы одна из проверок не пройдена</returns>
BOOL check_filenames_2(char *fn1, char *fn2);

/// <summary>
/// Проверяет имена файлов на правильность формата и совпадение
/// </summary>
/// <param name="fn1">Имя первого файла</param>
/// <param name="fn2">Имя второго файла</param>
/// <param name="fn3">Имя третьего файла</param>
/// <returns>1, если все проверки пройдены. 0, если хотя бы одна из проверок не пройдена</returns>
BOOL check_filenames_3(char *fn1, char *fn2, char *fn3);

/// <summary>
/// Проверка на существование файла (запись)
/// </summary>
/// <param name="filename">Имя файла в виде строки</param>
/// <returns>Указатель на файл</returns>
FILE *check_file_exist_write(char *filename);

/// <summary>
/// Проверка на существование файла (чтение)
/// </summary>
/// <param name="filename">Имя файла в виде строки</param>
/// <returns>Указатель на файл</returns>
FILE *check_file_exist_read(char *filename);

/// <summary>
/// Запись number в файл
/// </summary>
/// <param name="file">указатель на файл</param>
/// <param name="value">число</param>
void save_num_to_file(FILE *file, number *value);

/// <summary>
/// Чтение number из файла
/// </summary>
/// <param name="file">указатель на файл</param>
/// <param name="value">число</param>
int read_num_from_file(FILE *file, number *value);

/// <summary>
/// Сохраняет ключ
/// </summary>
/// <param name="filename">имя файла</param>
/// <param name="mod">N</param>
/// <param name="subkey">второй параметр (e или d)</param>
void save_key(char *filename, number *mod, number *subkey, char log);

/// <summary>
/// Читает ключ из файла
/// </summary>
/// <param name="filename">имя файла</param>
/// <param name="mod">N</param>
/// <param name="subkey">второй параметр (e или d)</param>
void read_key(char *filename, number *mod, number *subkey, char log);

/// <summary>
/// Function, returning a prime number from file.
/// </summary>
/// <param name="filename">char array contains filename</param>
/// <param name="line_number">number of prime number. can be one from 1-500</param>
/// <param name="bit_size">bit size of prime number</param>
/// <returns></returns>
number get_prime_from_file(char *filename, int line_number, int bit_size);

#endif // !_FILE__H
