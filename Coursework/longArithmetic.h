#pragma once
#include "config.h"

/// <summary>
/// ���������, ����������� ������������� ��� � ���������� �������� ������
/// </summary>
typedef struct
{
    int current_count; // ������� ���������� ���������
    int size; // �������� ������ �������
    char negative; // 1 ���� ����� �������������, -1 ���� �������������
    unsigned short* mas; // ������ ������
}number;

/// <summary>
/// �������������� ������ ��������� � ������ �������� � ������������� negative
/// </summary>
/// <returns>������ ���������</returns>
number init();

/// <summary>
/// ���������� ��������� ���������� ������
/// </summary>
/// <param name="value">������ �� ���������</param>
/// <returns>������ ���������, ���������� �������</returns>
number copy(number* value);

/// <summary>
/// ���������� number-������ ��� ���������� �����
/// </summary>
/// <param name="value">������ �� ������</param>
/// <returns>������</returns>
number normalize(number* value);

/// <summary>
/// ���������� ���������, � ������� �������� ������ ����� � ��������� int
/// </summary>
/// <param name="value">����� �����</param>
/// <returns>������ ���������, ���������� ������ �����</returns>
number int_to_number(int value);

/// <summary>
/// ������������ ����������� ���������� ������
/// </summary>
void clear_mem(number* value);

/// <summary>
/// ��������� char-������� � ����� ������� ��������� (����� ������ �����)
/// </summary>
/// <param name="object">������ �� ���������</param>
/// <param name="value">��������</param>
void add_digit(number* object, unsigned short value);

/// <summary>
/// �������� ��������� ����� (�������� �� ���� ������ ������)
/// </summary>
void offset_right(number* object);

/// <summary>
/// ���������� ���� � ����� (�������� �� ���� ������ �����)
/// </summary>
void offset_left(number* object);

/// <summary>
/// ������������� ����� � ������ �������
/// </summary>
/// <param name="value">������ �� �����</param>
void reverse(number* value);

/// <summary>
/// �������� ������ � ������ �����
/// </summary>
/// <param name="value">������ �� �����</param>
void print_number(number* value); 

/// <summary>
/// �������� ���� ����� �� �������, � ������ ����������� ����� ������ �� ���� �����, � ���������� ��������� ���� number
/// </summary>
/// <param name="value1">������ �����</param>
/// <param name="value2">������ �����</param>
/// <returns>��������� ���������</returns>
number difference(number* value1, number* value2);

/// <summary>
/// ���������� ���� ����� � �������, � ������ ����������� ����� ������ �� ���� �����, � ���������� ��������� ���� number
/// </summary>
/// <param name="value1">������ �����</param>
/// <param name="value2">������ �����</param>
/// <returns>��������� ��������</returns>
number addition(number* value1, number* value2);

/// <summary>
/// �������� ���� ����� �� ������ � ���������� ��������� ���� number
/// </summary>
/// <param name="value1">������ �����</param>
/// <param name="value2">������ �����</param>
/// <returns>��������� ���������</returns>
number multiplication(number* value1, number* value2);

/// <summary>
/// �������� ���� ����� �� ���� ����� � ���������� ��������� ���� number
/// </summary>
/// <param name="value1">������ �����</param>
/// <param name="value2">�����</param>
/// <returns>��������� ���������</returns>
number mult_to_digit(number* value1, int value2);

/// <summary>
/// �������� ����� � �������
/// </summary>
/// <param name="value1">������ �����</param>
/// <param name="value2">������� �����</param>
/// <returns>��������� ����������</returns>
number degree(number* value1, number* value2);

number division_with_remainder(number* value1, number* value2, number* ost);