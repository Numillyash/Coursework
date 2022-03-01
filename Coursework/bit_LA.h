#pragma once
#include "config.h"

#define NUMBER_SYSTEM_BASE 2

/// <summary>
/// ���������, ����������� ������������� ��� � ���������� �������� ������
/// </summary>
typedef struct
{
    int current_count; // ������� ���������� ���������
    int size; // �������� ������ �������
    uint8_t* mas; // ������ ������
}number;

/// <summary>
/// ��������� ����������� ���
/// </summary>
/// <param name="a">������ ���</param>
/// <param name="b">������ ���</param>
/// <returns>��������� ��������</returns>
inline uint8_t XOR(uint8_t a, uint8_t b);

/// <summary>
/// ��������� ��
/// </summary>
/// <param name="a">��� �����</param>
/// <returns>��������� ��������</returns>
inline uint8_t NOT(uint8_t a);

/// <summary>
/// ��������� �
/// </summary>
/// <param name="a">������ ���</param>
/// <param name="b">������ ���</param>
/// <returns>��������� ��������</returns>
inline uint8_t AND(uint8_t a, uint8_t b);

/// <summary>
/// ������� ����� � �������������� ���
/// </summary>
/// <param name="value"></param>
void additional_code(number* value);

/// <summary>
/// ������� ����� �� ��������������� � ������ ���
/// </summary>
/// <param name="value"></param>
void nonadditional_code(number* value);

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
void normalize(number* value);

/// <summary>
/// ������������ ����������� ���������� ������
/// </summary>
void clear_mem(number* value);

/// <summary>
/// ��������� char-������� � ����� ������� ��������� (����� ������ �����)
/// </summary>
/// <param name="object">������ �� ���������</param>
/// <param name="value">��������</param>
void add_digit(number* object, uint8_t value);

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
/// ��������� ����� � 0 (����� ����/�� ����� ����)
/// </summary>
/// <param name="object">�����, ������� ���������� ���������</param>
/// <returns>��������� ���������</returns>
BOOL is_zero(number* object);

/// <summary>
/// ��������� ����� � ������ ����� (�����/�� �����)
/// </summary>
/// <param name="value1">������ �����, ������� ���������� ��������</param>
/// <param name="value2">������ �����, ������� ���������� ��������</param>
/// <returns>��������� ���������</returns>
BOOL is_equal(number* value1, number* value2);

/// <summary>
/// ���������� ���� ����� � �������, � ������ ����������� ����� ������ �� ���� �����, � ���������� ��������� ���� number
/// </summary>
/// <param name="value1">������ �����</param>
/// <param name="value2">������ �����</param>
/// <returns>��������� ��������</returns>
number addition(number* value1, number* value2);

/// <summary>
/// �������� ���� ����� �� �������, � ������ ����������� ����� ������ �� ���� �����, � ���������� ��������� ���� number
/// </summary>
/// <param name="value1">������ �����</param>
/// <param name="value2">������ �����</param>
/// <returns>��������� ���������</returns>
number difference(number* value1, number* value2);

/// <summary>
/// �������� ���� ����� �� ������ � ���������� ��������� ���� number
/// </summary>
/// <param name="value1">������ �����</param>
/// <param name="value2">������ �����</param>
/// <returns>��������� ���������</returns>
number multiplication(number* value1, number* value2);

/// <summary>
/// ������� ��� �������
/// </summary>
/// <param name="value1">�������</param>
/// <param name="value2">��������</param>
/// <returns>�������</returns>
number division(number* value1, number* value2);

/// <summary>
/// ������� � ��������
/// </summary>
/// <param name="value1">�������</param>
/// <param name="value2">��������</param>
/// <param name="ost">�������</param>
/// <returns>�������</returns>
number division_with_remainder(number* value1, number* value2, number* ost);
