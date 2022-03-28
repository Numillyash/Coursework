#include "_file_.h"

FILE* check_file_exist_write(char* filename)
{
	FILE* file;
	file = fopen(filename, "w");

	if (file == NULL)
	{
		_log("Error while saving open key file");
		exit(FILE_OPEN_FAILURE);
	}
	return file;
}

FILE* check_file_exist_read(char* filename)
{
	FILE* file;
	file = fopen(filename, "r");

	if (file == NULL)
	{
		_log("Error while saving open key file");
		exit(FILE_OPEN_FAILURE);
	}
	return file;
}

//TODO: запихнуть чтение из файла ключа как юнион
// »де€
// символ читаетс€ как число в 4 бита
// тогда из двоичного числа можно получать сразу биты, мину€ перевод в 2сс
//

void save_num_to_file(FILE* file, number* value)
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

int read_num_from_file(FILE* file, number* value)
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

void save_key(char* filename, number* mod, number* subkey, char log) 
{
	FILE* file;
	char str[2];
	int i, j; // iterators 
	int c;
	file = check_file_exist_write(filename);

	fprintf(file, "_n_");
	while (((mod->current_count) - 1) % 4 != 0)
	{
		add_digit(mod, 0);
	}

	for (i = 0; i < mod->current_count - 1; i+=4)
	{
		c = 0;
		for (j = i+3; j >= i; j--)
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

void read_key(char* filename, number* mod, number* subkey, char log)
{
	FILE* file;
	char str[2];
	int i, j; // iterators 
	int c;
	int ms[4];
	
	*mod = init(); *subkey=init();
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
	_log("Open key parced succesfully. We read: ");
	str[0] = log;
	str[1] = '\0';
	_log(str);
}

number get_prime_from_file(char* filename, int line_number, int bit_size) 
{
	FILE* file; 
	int c = 0, line_num;
	char str[3];
	number res = init();

	file = fopen(filename, "r");
	if (file == NULL)
	{
		_log("Error while opening primes file");
		exit(FILE_OPEN_FAILURE);
	}
	while (c != (int)'&')
	{
		c = fgetc(file);
		if (c == (int)'_')
		{
			line_num = 0;
			c = fgetc(file);
			while (c != (int)'_')
			{
				line_num *= 10;
				line_num += (int)(c - '0');
				c = fgetc(file);
			}
			c = fgetc(file);
			if (line_num != line_number)
			{
				while (c != (int)'#')
				{
					c = fgetc(file);
				}
			}
			else
			{
				while (c != (int)'#')
				{
					add_digit(&res, (int)(c - '0'));
					c = fgetc(file);
				}
				if (res.current_count-1 != bit_size)
				{
					_log("Doesnt got prime number succesfully. Bit size doesnt match");
					fclose(file);
					exit(GET_PRIME_FAILURE);
				}
				_log("Got prime number succesfully");
				fclose(file);
				return res;
			}
		}
	}

	sprintf(str, "%d", line_number);
	_log("Doesnt got prime number succesfully. No prime. Line number was: ");
	_log(str);
	fclose(file);
	exit(GET_PRIME_FAILURE);
}