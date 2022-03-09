#include "_file_.h"

number get_prime_from_file(char* filename, int line_number, int bit_size) 
{
	FILE* file; int c = 0; int line_num; number res = init();
	file = fopen(filename, "r");
	if (file == NULL)
	{
		_log("Error while opening primes file");
		exit(FILE_OPENING_ERROR);
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
	_log("Doesnt got prime number succesfully. No prime");
	fclose(file);
	exit(GET_PRIME_FAILURE);
}