#include "_file_.h"

BOOL check_filenames_2(char *fn1, char *fn2)
{
	int strln;
	int i; // iterator
	char resol[5];
	resol[4] = '\0';
	if (!strcmp(fn1, fn2))
	{
		printf(ERROR_FILE_IDENT);
		_log("Files had same names");
		exit(FILE_FORMAT_FAILURE);
	}
	if (strlen(fn1) < 6 || strlen(fn2) < 6)
	{
		printf(ERROR_FILE_EXTENSION);
		_log("1 or more files had less then 5 symbols in filename");
		exit(FILE_FORMAT_FAILURE);
	}

	strln = strlen(fn1);
	for (i = strln - 4; i < strln; i++)
	{
		resol[4 - (strln - i)] = fn1[i];
	}
	if (strcmp(".txt", resol))
	{
		printf(ERROR_FILE_EXTENSION);
		_log("File has wrong extension");
		exit(FILE_FORMAT_FAILURE);
	}

	strln = strlen(fn2);
	for (i = strln - 4; i < strln; i++)
	{
		resol[4 - (strln - i)] = fn2[i];
	}
	if (strcmp(".txt", resol))
	{
		printf(ERROR_FILE_EXTENSION);
		_log("File has wrong extension");
		exit(FILE_FORMAT_FAILURE);
	}
	_log("Filenames had good format");
	return 1;
}

BOOL check_filenames_3(char *fn1, char *fn2, char *fn3)
{
	int strln;
	int i; // iterator
	char resol[5];
	resol[4] = '\0';
	if (!strcmp(fn1, fn2) || !strcmp(fn2, fn3) || !strcmp(fn1, fn3))
	{
		printf(ERROR_FILE_IDENT);
		_log("Files had same names");
		exit(FILE_FORMAT_FAILURE);
	}
	if (strlen(fn1) < 6 || strlen(fn2) < 6 || strlen(fn3) < 6)
	{
		printf(ERROR_FILE_EXTENSION);
		_log("1 or more files had less then 5 symbols in filename");
		exit(FILE_FORMAT_FAILURE);
	}

	strln = strlen(fn1);
	for (i = strln - 4; i < strln; i++)
	{
		resol[4 - (strln - i)] = fn1[i];
	}
	if (strcmp(".txt", resol))
	{
		printf(ERROR_FILE_EXTENSION);
		_log("File has wrong extension");
		exit(FILE_FORMAT_FAILURE);
	}

	strln = strlen(fn2);
	for (i = strln - 4; i < strln; i++)
	{
		resol[4 - (strln - i)] = fn2[i];
	}
	if (strcmp(".txt", resol))
	{
		printf(ERROR_FILE_EXTENSION);
		_log("File has wrong extension");
		exit(FILE_FORMAT_FAILURE);
	}

	strln = strlen(fn3);
	for (i = strln - 4; i < strln; i++)
	{
		resol[4 - (strln - i)] = fn3[i];
	}
	if (strcmp(".txt", resol))
	{
		printf(ERROR_FILE_EXTENSION);
		_log("File has wrong extension");
		exit(FILE_FORMAT_FAILURE);
	}

	_log("Filenames had good format");
	return 1;
}

FILE *check_file_exist_write(char *filename)
{
	FILE *file;
	file = fopen(filename, "w");

	if (file == NULL)
	{
		_log("Error while reading file (write):");
		_log(filename);
		exit(FILE_OPEN_FAILURE);
	}
	return file;
}

FILE *check_file_exist_read(char *filename)
{
	FILE *file;
	file = fopen(filename, "r");

	if (file == NULL)
	{
		_log("Error while reading file (read):");
		_log(filename);
		exit(FILE_OPEN_FAILURE);
	}
	return file;
}

void save_num_to_file(FILE *file, number *value)
{
	int i, c, j;

	fprintf(file, "_c_");
	while (((value->current_count) - 1) % 4 != 0)
	{
		add_digit(value, 0);
	}

	for (i = 0; i < value->current_count - 1; i += 4)
	{
		c = 0;
		for (j = i + 3; j >= i; j--)
		{
			c = c * 2 + value->mas[j];
		}
		fprintf(file, "%c", (char)(c + (int)'a'));
	}

	fprintf(file, "#\n");
}

void check_readed_num(char *string)
{
	char buff[PREFIX_SIZE + 1];
	int i; // iterator
	int str_len;
	char check_str_min = 'a', check_str_max = 'a' + 15;
	BOOL result = FALSE;

	buff[PREFIX_SIZE] = '\0';
	for (i = 0; i < PREFIX_SIZE; i++)
	{
		buff[i] = string[i];
	}
	if (!strcmp("_n_", buff) || !strcmp("_d_", buff) || !strcmp("_e_", buff) || !strcmp("_c_", buff))
		result = TRUE;
	if (!result)
	{
		_log("Wrong number prefix");
		exit(GET_NUMBER_FAILURE);
	}

#ifdef _WIN32
	str_len = strlen(string) - 2;
#else
	str_len = strlen(string) - 2;
#endif
	if (string[str_len] != '#')
	{
		_log("End of number doesnt exist");
		exit(GET_NUMBER_FAILURE);
	}

	for (i = 3; i < str_len; i++)
	{
		if (string[i] < check_str_min || string[i] > check_str_max)
		{
			_log("There's exist some unexpected symbol");
			exit(GET_NUMBER_FAILURE);
		}
	}
}

int read_num_from_file(FILE *file, number *value)
{
	int i, j; // iterators
	int c;
	int ms[4];
	char buff[2048];

	fgets(buff, 2048, file);
	if (!strcmp("EOF", buff))
	{
		return 0;
	}
	check_readed_num(buff);

	i = 3;
	while ((c = (int)buff[i]) != (int)'#')
	{
		c = (int)(buff[i] - 'a');
		for (j = 0; j < 4; j++)
		{
			ms[j] = c % 2;
			c >>= 1;
		}
		for (j = 0; j < 4; j++)
		{
			add_digit(value, ms[j]);
		}
		i++;
	}
	return 1;
}

void save_key(char *filename, number *mod, number *subkey, char log)
{
	FILE *file;
	char str[2];
	int i, j; // iterators
	int c;
	file = check_file_exist_write(filename);

	fprintf(file, "_n_");
	while (((mod->current_count) - 1) % 4 != 0)
	{
		add_digit(mod, 0);
	}

	for (i = 0; i < mod->current_count - 1; i += 4)
	{
		c = 0;
		for (j = i + 3; j >= i; j--)
		{
			c = c * 2 + mod->mas[j];
		}
		fprintf(file, "%c", (char)(c + (int)'a'));
	}

	fprintf(file, "#\n_%c_", log);
	while (((subkey->current_count) - 1) % 4 != 0)
	{
		add_digit(subkey, 0);
	}

	for (i = 0; i < subkey->current_count - 1; i += 4)
	{
		c = 0;
		for (j = i + 3; j >= i; j--)
		{
			c = c * 2 + subkey->mas[j];
		}
		fprintf(file, "%c", (char)(c + (int)'a'));
	}
	fprintf(file, "#\n&");

	fclose(file);

	_log("Key saved succesfully. Subkey was: ");
	str[0] = log;
	str[1] = '\0';
	_log(str);
}

void read_key(char *filename, number *mod, number *subkey, char log)
{
	FILE *file;
	char str[2];
	int i, j; // iterators
	int c;
	int ms[4];

	*mod = init();
	*subkey = init();
	file = check_file_exist_read(filename);

	char buff[2048];

	fgets(buff, 2048, file);

	i = 3;
	while ((c = (int)buff[i]) != (int)'#')
	{
		c = (int)(buff[i] - 'a');
		for (j = 0; j < 4; j++)
		{
			ms[j] = c % 2;
			c >>= 1;
		}
		for (j = 0; j < 4; j++)
		{
			add_digit(mod, ms[j]);
		}
		i++;
	}

	fgets(buff, 2048, file);

	i = 3;
	while ((c = (int)buff[i]) != (int)'#')
	{
		c = (int)(buff[i] - 'a');
		// TODO Оптимизировать цикл - избавиться от цикла
		for (j = 0; j < 4; j++)
		{
			ms[j] = c % 2;
			c >>= 1;
		}
		for (j = 0; j < 4; j++)
		{
			add_digit(subkey, ms[j]);
		}
		i++;
	}

	normalize(mod);
	normalize(subkey);
	fclose(file);
	_log("Key parced succesfully. We read: ");
	str[0] = log;
	str[1] = '\0';
	_log(str);
}

void check_readed_prime(char *string)
{
	char buff[PRIME_PREFIX_SIZE + 1];
	int i; // iterator
	int str_len;
	char check_str_min = '0', check_str_max = '0' + 1;
	char check_prx_min = '0', check_prx_max = '0' + 9;
	BOOL result = FALSE;

	if (strcmp("&", string))
	{
		buff[PRIME_PREFIX_SIZE] = '\0';
		for (i = 0; i < PRIME_PREFIX_SIZE; i++)
		{
			buff[i] = string[i];
		}
		if (buff[0] != '_')
		{
			_log("Wrong prime number prefix:");
			_log(buff);
			exit(GET_PRIME_FAILURE);
		}
		i--;
		while (buff[i] != '_')
		{
			buff[i] = '\0';
			i--;
		}
		str_len = strlen(buff);
		if (str_len < 3)
		{
			_log("Wrong prime number prefix:");
			_log(buff);
			snprintf(buff, 5, "%d", str_len);
			_log(buff);
			exit(GET_PRIME_FAILURE);
		}
		for (i = 1; i < str_len - 1; i++)
		{
			if (string[i] < check_prx_min || string[i] > check_prx_max)
			{
				_log("There's exist some unexpected symbol");
				_log(buff);
				buff[1] = '\0';
				buff[0] = string[i];
				_log(buff);
				snprintf(buff, 5, "%d", i);
				_log(buff);
				exit(GET_PRIME_FAILURE);
			}
		}

#ifdef _WIN32
		str_len = strlen(string) - 2;
#else
		str_len = strlen(string) - 2;
#endif
		if (string[str_len] != '#')
		{
			_log("End of number doesnt exist");
			buff[1] = '\0';
			buff[0] = string[str_len];
			_log(buff);
			snprintf(buff, 5, "%d", str_len);
			_log(buff);
			snprintf(buff, 5, "%d", (int)string[str_len]);
			_log(buff);
			exit(GET_PRIME_FAILURE);
		}

		for (i = strlen(buff); i < str_len; i++)
		{
			if (string[i] < check_str_min || string[i] > check_str_max)
			{
				_log("There's exist some unexpected symbol");
				exit(GET_PRIME_FAILURE);
			}
		}
	}
}

number get_prime_from_file(char *filename, int line_number, int bit_size)
{
	FILE *file;
	int c = 0, line_num, i = 0;
	char str[3];
	number res = init();
	char buff[4096];

	file = check_file_exist_read(filename);

	fgets(buff, 4096, file);
	check_readed_prime(buff);
	while (strcmp("&", buff))
	{
		i = 1;
		c = buff[i];
		line_num = 0;
		while (c != (int)'_')
		{
			line_num *= 10;
			line_num += (int)(c - '0');
			i++;
			c = buff[i];
		}
		i++;
		c = buff[i];
		if (line_num == line_number)
		{
			while (c != (int)'#')
			{
				add_digit(&res, (int)(c - '0'));
				i++;
				if (i > 2100)
				{
					_log("Doesnt got prime number succesfully. Bit size doesnt match");
					fclose(file);
					exit(GET_PRIME_FAILURE);
				}
				c = buff[i];
			}
			if (res.current_count - 1 != bit_size)
			{
				_log("Doesnt got prime number succesfully. Bit size doesnt match");
				fclose(file);
				exit(GET_PRIME_FAILURE);
			}
			_log("Got prime number succesfully");
			debug_log(&res);
			fclose(file);
			return res;
		}

		fgets(buff, 4096, file);
		check_readed_prime(buff);
	}

	sprintf(str, "%d", line_number);
	_log("Doesnt got prime number succesfully. No prime. Line number was: ");
	_log(str);
	fclose(file);
	exit(GET_PRIME_FAILURE);
}
