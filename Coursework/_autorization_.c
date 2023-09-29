#include "_autorization_.h"

const uint8_t *correct_pswd = "crack422214014this";
void convert_to_CRC(uint8_t *input, uint64_t input_size, uint8_t *output, uint32_t output_size)
{
    uint64_t i, j;
    // char crc_bin[33];

    uint32_t crc_table[256];
    uint32_t crc;

    for (i = 0; i < 256; i++)
    {
        crc = i;
        for (j = 0; j < 8; j++)
            crc = crc & 1 ? (crc >> 1) ^ CRC_code_k : crc >> 1;

        crc_table[i] = crc;
    }

    crc = 0; // xFFFFFFFFUL;
    while (input_size--)
    {
        i = (crc ^ *input++) & 0xFF;
        crc = crc_table[i] ^ (crc >> 8);
    }
    crc ^= 0; // xFFFFFFFFUL;

    //_itoa(crc, crc_bin, 2);
    snprintf(output, output_size, "KEY$%x$\n", crc);
}

void check_password()
{
    FILE *pswd_file = check_file_exist_read(PASSWORD_FILE_NAME);
    FILE *key_file = check_file_exist_write(KEY_FILE_NAME);
    char buff[2048];
    int i;

    fgets(buff, 2048, pswd_file);
    if (!strcmp("EOF", buff))
    {
        _log("Error while reading password (null file):");
        _log(buff);
        exit(PASSWORD_NULL_FAILURE);
    }
    i = 0;
    while (buff[i] != '\n')
    {
        if (i > 1000)
        {
            _log("Error while reading password (too long password):");
            _log(buff);
            exit(PASSWORD_LONG_FAILURE);
        }
        i++;
    }
    char *entered_pswd = (char *)malloc(i + 1);
    strcpy_s(entered_pswd, i, buff);
    entered_pswd[i] = '\n';
    if (!strcmp(entered_pswd, correct_pswd))
    {
        _log("Incorrect password:");
        _log(buff);
        exit(INCORRECT_PASSWORD_FAILURE);
    }
    char crc_result[14];
    convert_to_CRC(buff, i + 1, crc_result, 14);
    fprintf_s(key_file, "%s", crc_result);
}