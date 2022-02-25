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
/// Возвращает number-обьект без незначащих нулей
/// </summary>
/// <param name="value">Ссылка на обьект</param>
/// <returns>Обьект</returns>
number normalize(number* value);

/// <summary>
/// Возвращает структуру, в которой записано данное число в диапазоне int
/// </summary>
/// <param name="value">Целое число</param>
/// <returns>Обьект структуры, содержащий данное число</returns>
number int_to_number(int value);

/// <summary>
/// Освобождение динамически выделенной памяти
/// </summary>
void clear_mem(number* value);

/// <summary>
/// Добавляет char-элемент в конец массива структуры (новый разряд слева)
/// </summary>
/// <param name="object">Ссылка на структуру</param>
/// <param name="value">Значение</param>
void add_digit(number* object, unsigned short value);

/// <summary>
/// Удаление последней цифры (смещение на один разряд вправо)
/// </summary>
void offset_right(number* object);

/// <summary>
/// Добавление нуля в конец (смещение на один разряд влево)
/// </summary>
void offset_left(number* object);

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
number difference(number* value1, number* value2);

/// <summary>
/// Прибавляет одно число к другому, с учетом внутреннего знака данных на вход чисел, и возвращает результат типа number
/// </summary>
/// <param name="value1">Первое число</param>
/// <param name="value2">Второе число</param>
/// <returns>Результат сложения</returns>
number addition(number* value1, number* value2);

/// <summary>
/// Умножает одно число на другое и возвращает результат типа number
/// </summary>
/// <param name="value1">Первое число</param>
/// <param name="value2">Второе число</param>
/// <returns>Результат умножения</returns>
number multiplication(number* value1, number* value2);

/// <summary>
/// Умножает одно число на одну цифру и возвращает результат типа number
/// </summary>
/// <param name="value1">Первое число</param>
/// <param name="value2">Цифра</param>
/// <returns>Результат умножения</returns>
number mult_to_digit(number* value1, int value2);

/// <summary>
/// Возводит число в степень
/// </summary>
/// <param name="value1">Первое число</param>
/// <param name="value2">Степень числа</param>
/// <returns>Результат возведения</returns>
number degree(number* value1, number* value2);

number division_with_remainder(number* value1, number* value2, number* ost);