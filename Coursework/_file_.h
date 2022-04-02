#ifndef _FILE__H
#define _FILE__H

#include "config.h"
#include "bit_LA.h"

#define PREFIX_SIZE 3
#define PRIME_PREFIX_SIZE 5

/// <summary>
/// ��������� ����� ������ �� ������������ ������� � ����������
/// </summary>
/// <param name="fn1">��� ������� �����</param>
/// <param name="fn2">��� ������� �����</param>
/// <returns>1, ���� ��� �������� ��������. 0, ���� ���� �� ���� �� �������� �� ��������</returns>
BOOL check_filenames_2(char* fn1, char* fn2);

/// <summary>
/// ��������� ����� ������ �� ������������ ������� � ����������
/// </summary>
/// <param name="fn1">��� ������� �����</param>
/// <param name="fn2">��� ������� �����</param>
/// <param name="fn3">��� �������� �����</param>
/// <returns>1, ���� ��� �������� ��������. 0, ���� ���� �� ���� �� �������� �� ��������</returns>
BOOL check_filenames_3(char* fn1, char* fn2, char* fn3);

/// <summary>
/// �������� �� ������������� ����� (������)
/// </summary>
/// <param name="filename">��� ����� � ���� ������</param>
/// <returns>��������� �� ����</returns>
FILE* check_file_exist_write(char* filename);

/// <summary>
/// �������� �� ������������� ����� (������)
/// </summary>
/// <param name="filename">��� ����� � ���� ������</param>
/// <returns>��������� �� ����</returns>
FILE* check_file_exist_read(char* filename);

/// <summary>
/// ������ number � ����
/// </summary>
/// <param name="file">��������� �� ����</param>
/// <param name="value">�����</param>
void save_num_to_file(FILE* file, number* value);

/// <summary>
/// ������ number �� �����
/// </summary>
/// <param name="file">��������� �� ����</param>
/// <param name="value">�����</param>
int read_num_from_file(FILE* file, number* value);

/// <summary>
/// ��������� ����
/// </summary>
/// <param name="filename">��� �����</param>
/// <param name="mod">N</param>
/// <param name="subkey">������ �������� (e ��� d)</param>
void save_key(char* filename, number* mod, number* subkey, char log);

/// <summary>
/// ������ ���� �� �����
/// </summary>
/// <param name="filename">��� �����</param>
/// <param name="mod">N</param>
/// <param name="subkey">������ �������� (e ��� d)</param>
void read_key(char* filename, number* mod, number* subkey, char log);

/// <summary>
/// Function, returning a prime number from file.
/// </summary>
/// <param name="filename">char array contains filename</param>
/// <param name="line_number">number of prime number. can be one from 1-500</param>
/// <param name="bit_size">bit size of prime number</param>
/// <returns></returns>
number get_prime_from_file(char* filename, int line_number, int bit_size);

#endif // !_FILE__H
