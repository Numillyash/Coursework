#include "RSA.h"

/// <summary>
/// Функция для нахождения остатка от деления большого числа на малое
/// </summary>
/// <param name="a">Делимое</param>
/// <param name="b">Делитель</param>
/// <returns>Остаток</returns>
int module_op(double a, int b)
{
    double c = a / b;

    modf(c, &c);
    a -= (double)b * c;
    if (a == 0)
        return 0;
    else
        return ((int)a < 0) ? (int)a + b : (int)a;
}



void generate_key(char* key_size_str, char* pubkey_filename, char* seckey_filename)
{
    FILE* pubkey, * seckey;
    KEY_BIT_SIZE keysize; int key_s_buf = 0;

    pubkey = fopen(pubkey_filename, "w");
    seckey = fopen(seckey_filename, "w");

    if (pubkey == NULL)
    {
        printf(FILE_OPENING_ERROR, pubkey_filename);
        exit(FILE_OPEN_ERROR);
    }
    if (seckey == NULL)
    {
        printf(FILE_OPENING_ERROR, seckey_filename);
        exit(FILE_OPEN_ERROR);
    }

    key_s_buf = atoi(key_size_str);
    if (key_s_buf == 0)
    {
        printf("Wrong key size: %s.\nType -h for help", key_size_str);
        exit(DEFAULT_ERROR);
    }
    else
        switch (key_s_buf)
        {
        case KEY_256:
            keysize = KEY_256;
            break;
        case KEY_512:
            keysize = KEY_512;
            break;
        case KEY_1024:
            keysize = KEY_1024;
            break;
        case KEY_2048:
            keysize = KEY_2048;
            break;
        default:
            printf("Wrong key size: %s.\nType -h for help", key_size_str);
            exit(DEFAULT_ERROR);
            break;
        }




    fclose(pubkey); fclose(seckey);
}