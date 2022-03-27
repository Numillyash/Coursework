#pragma once
#include "config.h"
#include "bit_LA.h"

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

void save_open_key(char* filename, number* mod, number* pubkey);

void save_secret_key(char* filename, number* seckey);

/// <summary>
/// ������ �������� ���� �� �����
/// </summary>
/// <param name="filename">��� �����</param>
/// <param name="mod">��������� �� �������������������� �����</param>
/// <param name="pubkey">��������� �� �������������������� �����</param>
void read_open_key(char* filename, number* mod, number* pubkey);

void read_secret_key(char* filename, number* seckey);

number get_prime_from_file(char* filename, int line_number, int bit_size);