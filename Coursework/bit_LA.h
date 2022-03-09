#pragma once
#include "config.h"
#include "_log_.h"

#define NUMBER_SYSTEM_BASE 2
#define MILLERS_METHOD_ITERATIONS_NUMBER 20

/// <summary>
/// Структура, описывающая целочисленный тип с изменяемым размером памяти
/// </summary>
typedef struct
{
    int current_count; // текущее количество элементов
    int size; // реальный размер массива
    uint8_t* mas; // массив данных
}number;

/// <summary>
/// Побитовое исключающее ИЛИ
/// </summary>
/// <param name="a">Первый бит</param>
/// <param name="b">Второй бит</param>
/// <returns>Результат операции</returns>
inline uint8_t XOR(uint8_t a, uint8_t b);

/// <summary>
/// Побитовое НЕ
/// </summary>
/// <param name="a">Бит числа</param>
/// <returns>Результат операции</returns>
inline uint8_t NOT(uint8_t a);

/// <summary>
/// Побитовое И
/// </summary>
/// <param name="a">Первый бит</param>
/// <param name="b">Второй бит</param>
/// <returns>Результат операции</returns>
inline uint8_t AND(uint8_t a, uint8_t b);

/// <summary>
/// Перевод числа в дополнительный код
/// </summary>
/// <param name="value"></param>
void additional_code(number* value);

/// <summary>
/// Перевод числа из дополнительного в прямой код
/// </summary>
/// <param name="value"></param>
void nonadditional_code(number* value);

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
void normalize(number* value);

/// <summary>
/// Освобождение динамически выделенной памяти
/// </summary>
void clear_mem(number* value);

/// <summary>
/// Добавляет char-элемент в конец массива структуры (новый разряд слева)
/// </summary>
/// <param name="object">Ссылка на структуру</param>
/// <param name="value">Значение</param>
void add_digit(number* object, uint8_t value);

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
/// Печатает обьект с учетом знака в десятичной СС
/// </summary>
/// <param name="value">Ссылка на число</param>
void print_number_decimal(number* value);

/// <summary>
/// Печатает обьект с учетом знака
/// </summary>
/// <param name="value">Ссылка на число</param>
void print_number(number* value);

/// <summary>
/// Сравнение числа с 0 (Равно нулю/не равно нулю)
/// </summary>
/// <param name="object">Число, которое необходимо проверить</param>
/// <returns>Результат сравнения</returns>
BOOL is_zero(number* object);

/// <summary>
/// Сравнение числа с другим число (Равны/не равны)
/// </summary>
/// <param name="value1">Первое число, которое необходимо сравнить</param>
/// <param name="value2">Второе число, которое необходимо сравнить</param>
/// <returns>Результат сравнения</returns>
BOOL is_equal(number* value1, number* value2);

/// <summary>
/// Прибавляет одно число к другому, с учетом внутреннего знака данных на вход чисел, и возвращает результат типа number
/// </summary>
/// <param name="value1">Первое число</param>
/// <param name="value2">Второе число</param>
/// <returns>Результат сложения</returns>
number addition(number* value1, number* value2);

/// <summary>
/// Вычитает одно число из другого, с учетом внутреннего знака данных на вход чисел, и возвращает результат типа number
/// </summary>
/// <param name="value1">Первое число</param>
/// <param name="value2">Второе число</param>
/// <returns>Результат вычитания</returns>
number difference(number* value1, number* value2);

/// <summary>
/// Умножает одно число на другое и возвращает результат типа number
/// </summary>
/// <param name="value1">Первое число</param>
/// <param name="value2">Второе число</param>
/// <returns>Результат умножения</returns>
number multiplication(number* value1, number* value2);

/// <summary>
/// Деление с остатком
/// </summary>
/// <param name="value1">Делимое</param>
/// <param name="value2">Делитель</param>
/// <param name="ost">Остаток</param>
/// <returns>Частное</returns>
number division_with_module(number* value1, number* value2, number* ost);

/// <summary>
/// Возвращает модуль от деление числа a в степени t на b
/// </summary>
/// <param name="a">Делимое</param>
/// <param name="t">Степень</param>
/// <param name="b">Делитель</param>
/// <returns>Остаток</returns>
number module_pow(number* a, number* t, number* b);

/// <summary>
/// Возвращает НОД двух чисел
/// </summary>
/// <param name="value1">Первое число</param>
/// <param name="value2">Второе число</param>
/// <returns>НОД</returns>
number euclide_algorithm(number* value1, number* value2);

/// <summary>
/// Возвращает НОД двух чисел и матрицу множетелей
/// </summary>
/// <param name="value1">Первое число</param>
/// <param name="value2">Второе число</param>
/// <param name="values">Массив с матрицей</param>
/// <returns>НОД</returns>
number euclide_algorithm_modifyed(number* value1, number* value2, number* values);

/// <summary>
/// Генерация случайного числа с заданным количеством бит
/// </summary>
/// <param name="bit_count">Кол-во бит</param>
/// <returns>Сгенерированное число</returns>
number generate_random(int bit_count);

/// <summary>
/// Проверка числа на простоту
/// </summary>
/// <param name="value">Число</param>
/// <returns>1 - если простое, 0 - если составное</returns>
BOOL Millers_method(number* value);