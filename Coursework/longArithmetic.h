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
/// ���������� ���������, � ������� �������� ������ ����� � ��������� int
/// </summary>
/// <param name="value">����� �����</param>
/// <returns>������ ���������, ���������� ������ �����</returns>
number int_to_number(int value);

/// <summary>
/// ���������� number-������ ��� ���������� �����
/// </summary>
/// <param name="value">������ �� ������</param>
/// <returns>������</returns>
number normalize(number* value);

/// <summary>
/// ��������� char-������� � ����� ������� ���������
/// </summary>
/// <param name="object">������ �� ���������</param>
/// <param name="value">��������</param>
void add_element(number* object, unsigned char value);

void remove_last_digit(number* object);

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
number minus(number* value1, number* value2);

/// <summary>
/// ���������� ���� ����� � �������, � ������ ����������� ����� ������ �� ���� �����, � ���������� ��������� ���� number
/// </summary>
/// <param name="value1">������ �����</param>
/// <param name="value2">������ �����</param>
/// <returns>��������� ��������</returns>
number plus(number* value1, number* value2);

/// <summary>
/// �������� ���� ����� �� ������ � ���������� ��������� ���� number
/// </summary>
/// <param name="value1">������ �����</param>
/// <param name="value2">������ �����</param>
/// <returns>��������� ���������</returns>
number proizv(number* value1, number* value2); 

/// <summary>
/// �������� ���� ����� �� ���� ����� � ���������� ��������� ���� number
/// </summary>
/// <param name="value1">������ �����</param>
/// <param name="value2">�����</param>
/// <returns>��������� ���������</returns>
number proizv_to_digit(number* value1, int value2);

/// <summary>
/// �������� ����� � �������
/// </summary>
/// <param name="value1">������ �����</param>
/// <param name="value2">������� �����</param>
/// <returns>��������� ����������</returns>
number degree(number* value1, number* value2);

number deli_v_stolbik(number* value1, number* value2, number* ost);