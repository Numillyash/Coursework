#pragma once
#include "config.h"

/// <summary>
/// Структура, описывающая целочисленный тип с изменяемым размером памяти
/// </summary>
typedef struct
{
    int current_count; // текущее количество элементов
    int size; // реальный размер массива
    char negative; // 1 если число положительное, -1 если отрицательное
    unsigned short* mas; // массив данных
}number;

/// <summary>
/// Инициализирует обьект структуры с пустым массивом и положительным negative
/// </summary>
/// <returns>Обьект структуры</returns>
number init();

/// <summary>
/// Возвращает структуру идентичную данной
/// </summary>
/// <param name="value">Ссылка на структуру</param>
/// <returns>Обьект структуры, идентичный данному</returns>
number copy(number* value);

/// <summary>
/// Возвращает структуру, в которой записано данное число в диапазоне int
/// </summary>
/// <param name="value">Целое число</param>
/// <returns>Обьект структуры, содержащий данное число</returns>
number int_to_number(int value);

/// <summary>
/// Возвращает number-обьект без незначащих нулей
/// </summary>
/// <param name="value">Ссылка на обьект</param>
/// <returns>Обьект</returns>
number normalize(number* value);

/// <summary>
/// Добавляет char-элемент в конец массива структуры
/// </summary>
/// <param name="object">Ссылка на структуру</param>
/// <param name="value">Значение</param>
void add_element(number* object, unsigned char value);

void remove_last_digit(number* object);

/// <summary>
/// Разворачивает число в данном обьекте
/// </summary>
/// <param name="value">Ссылка на число</param>
void reverse(number* value);

/// <summary>
/// Печатает обьект с учетом знака
/// </summary>
/// <param name="value">Ссылка на число</param>
void print_number(number* value); 

/// <summary>
/// Вычитает одно число из другого, с учетом внутреннего знака данных на вход чисел, и возвращает результат типа number
/// </summary>
/// <param name="value1">Первое число</param>
/// <param name="value2">Второе число</param>
/// <returns>Результат вычитания</returns>
number minus(number* value1, number* value2);

/// <summary>
/// Прибавляет одно число к другому, с учетом внутреннего знака данных на вход чисел, и возвращает результат типа number
/// </summary>
/// <param name="value1">Первое число</param>
/// <param name="value2">Второе число</param>
/// <returns>Результат сложения</returns>
number plus(number* value1, number* value2);

/// <summary>
/// Умножает одно число на другое и возвращает результат типа number
/// </summary>
/// <param name="value1">Первое число</param>
/// <param name="value2">Второе число</param>
/// <returns>Результат умножения</returns>
number proizv(number* value1, number* value2); 

/// <summary>
/// Умножает одно число на одну цифру и возвращает результат типа number
/// </summary>
/// <param name="value1">Первое число</param>
/// <param name="value2">Цифра</param>
/// <returns>Результат умножения</returns>
number proizv_to_digit(number* value1, int value2);

/// <summary>
/// Возводит число в степень
/// </summary>
/// <param name="value1">Первое число</param>
/// <param name="value2">Степень числа</param>
/// <returns>Результат возведения</returns>
number degree(number* value1, number* value2);

number deli_v_stolbik(number* value1, number* value2, number* ost);