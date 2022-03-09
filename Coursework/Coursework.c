#include "config.h"
#include "help.h"

#include "_file_.h"

/// <summary>
/// Function that is checking for parameters for work mode
/// </summary>
/// <param name="argv">Input parameters from console</param>
/// <param name="work_mode">Expected work mode, that will be checked</param>
/// <param name="arg_1">Expected parameter 1</param>
/// <param name="arg_2">Expected parameter 2</param>
/// <param name="arg_3">Expected parameter 3</param>
/// <returns>0 if work mode isnt expected, 1 if parameters are right</returns>
int check_console_input_format(char** argv, char* work_mode, char* arg_1, char* arg_2, char* arg_3)
{
	if (!strcmp(argv[1], work_mode))
	{
		if (!strcmp(argv[2], arg_1))
		{
			if (!strcmp(argv[4], arg_2))
			{
				if (!strcmp(argv[6], arg_3))
				{
					return 1;
				}
				else
				{
					printf(WRONG_PARAMETER, argv[6], arg_3);
					exit(WRONG_ARGUMENT_FAILURE);
				}
			}
			else
			{
				printf(WRONG_PARAMETER, argv[4], arg_2);
				exit(WRONG_ARGUMENT_FAILURE);
			}
		}
		else
		{
			printf(WRONG_PARAMETER, argv[2], arg_1);
			exit(WRONG_ARGUMENT_FAILURE);
		}
	}
	else
		return 0;
}

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "rus");
	// брать p и q из базы простых чисел
	// параллельные вычисления OpenCL, CUDA
	_log_start();
	_log("thats a log");
	_log("thats an another log");

	//int i = 1100101, j = 0, l = 239;
	/*int p = 3557, q = 2579;
	number _p = int_to_number(p);
	number _q = int_to_number(q);
	int n = p * q; // 9 173 503
	int phi_n = (p - 1) * (q - 1); // 9 167 368
	number _phi_n = int_to_number(phi_n);
	int e = 67;
	number _e = int_to_number(e);

	number* a = (number*)malloc(4*sizeof(number)); // = { 1,0,0,1 };
	number zero = int_to_number(0);
	number one = int_to_number(1);

	a[0] = copy(&one);
	a[1] = copy(&zero);
	a[2] = copy(&zero);
	a[3] = copy(&one);

	number buff = euclide_algorithm_modifyed(&_phi_n, &_e, a);
	clear_mem(&buff);

	number _d = copy(&a[1]); // 2 462 875
	printf("e = %d\nd = %d\ne*d = %d\nphi_n = %d\n", e, 2462875, 165012625, phi_n);

	printf("_e = "); print_number_decimal(&_e);
	printf("_d = "); print_number_decimal(&_d);
	buff = multiplication(&_e, &_d);
	printf("_e*_d = "); print_number_decimal(&buff); clear_mem(&buff);
	printf("_phi_n = "); print_number_decimal(&_phi_n);*/

	/*
	number a = init();
	for (j = 2; j < 100; j++) {
		clear_mem(&a);
		a = int_to_number(j);
		//number mod = Millers_method(&a);
		//print_number(&div); print_number(&mod);
		BOOL s = Millers_method(&a);
		if (s)
		{
			printf("Number %d is prime\n", j);
		}
	}*/
	//print_number_decimal(&mod);


	// fwrite();


	/*int i = 2, j = 3;
	number a = int_to_number(i);
	number b = int_to_number(j);
	number mod;
	number div = division_with_module(&a, &b, &mod);
	//print_number(&div); print_number(&mod);
	printf("a = %d, b = %d, a/b = ", i, j);
	if (div.mas[div.current_count - 1])
	{
		printf("-");
		nonadditional_code(&div);
		int x = 1;
		int s = 0;
		for (int y = 0; y < div.current_count - 1; y++)
		{
			s += div.mas[y] * x;
			x <<= 1;
		}
		printf("%d a%%b = ", s);
	}
	else
	{
		int x = 1;
		int s = 0;
		for (int y = 0; y < div.current_count - 1; y++)
		{
			s += div.mas[y] * x;
			x <<= 1;
		}
		printf("%d a%%b = ", s);
	}
	if (!mod.mas[mod.current_count - 1])
	{
		int x = 1;
		int s = 0;
		for (int y = 0; y < mod.current_count - 1; y++)
		{
			s += mod.mas[y] * x;
			x <<= 1;
		}
		printf("%d\n", s);
	}
	*/

	/*int k = -1;
	for (int i = -k; i <= k; i++)
	{
		for (int j = -k; j <= k; j++)
		{
			number a = int_to_number(i); // 01111
			number b = int_to_number(j); // 11101
			number c = difference(&a, &b);
			printf("a = %d, b = %d, a-b = ", i, j);
			if (c.mas[c.current_count - 1])
			{
				printf("-");
				nonadditional_code(&c);
				int x = 1;
				int s = 0;
				for (int y = 0; y < c.current_count - 1; y++)
				{
					s += c.mas[y] * x;
					x <<= 1;
				}
				printf("%d\n", s);
			}
			else
			{
				int x = 1;
				int s = 0;
				for (int y = 0; y < c.current_count - 1; y++)
				{
					s += c.mas[y] * x;
					x <<= 1;
				}
				printf("%d\n", s);
			}
		}
	}
	*/
	//number c = addition(&a, &b);  // 1100
	//print_number(&c);
	//c = addition(&c, &c);
	//print_number(&c);


	/*
	clock_t start = clock();

	number n1 = int_to_number(1);
	for (size_t i = 0; i < 16; i++)
	{
		offset_left(&n1);
	}
	number n2 = copy(&n1);
	number n3 = int_to_number(239);

	clock_t end = clock();
	double seconds = (double)(end - start) / CLOCKS_PER_SEC;
	printf("The time: %f seconds\n", seconds);

	number n4 = module_pow(&n1,&n2,&n3);

	print_number(&n4);

	end = clock();
	seconds = (double)(end - start) / CLOCKS_PER_SEC;
	printf("The time: %f seconds\n", seconds);

	*/

	if (argc == 1)
	{
		printf(ERROR_NO_PARAMETERS);
		exit(NO_ARGUMENTS_FAILURE);
	}
	else if (argc == 2)
	{
		if (!strcmp(argv[1], HELP_CONSOLE_OPTION_1) || !strcmp(argv[1], HELP_CONSOLE_OPTION_2))
		{
			printf(HELP_TEXT);
			exit(WRONG_ARGUMENT_FAILURE);
		}
		else if (strcmp(argv[1], GENKEY_CONSOLE_OPTION) && strcmp(argv[1], CHECK_CONSOLE_OPTION) && strcmp(argv[1], SIGNATURE_CONSOLE_OPTION) && strcmp(argv[1], ENCRYPT_CONSOLE_OPTION) && strcmp(argv[1], DECRYPT_CONSOLE_OPTION))
		{
			printf(ERROR_FIRST_PARAMETER, argv[1]);
			exit(WRONG_ARGUMENT_FAILURE);
		}
		else
		{
			printf(ERROR_NOT_ENOUGH_PARAMETERS);
			exit(NOT_ENOUGH_ARGUMENTS_FAILURE);
		}
	}
	else if (argc == 8)
	{
		if (check_console_input_format(argv, GENKEY_CONSOLE_OPTION, SIZE_CONSOLE_OPTION, PUBKEY_CONSOLE_OPTION, SECRET_CONSOLE_OPTION))
		{
			//do some code
		}
		else if (check_console_input_format(argv, SIGNATURE_CONSOLE_OPTION, INFILE_CONSOLE_OPTION, SECRET_CONSOLE_OPTION, SIGFILE_CONSOLE_OPTION))
		{
			//do some code
		}
		else if (check_console_input_format(argv, CHECK_CONSOLE_OPTION, INFILE_CONSOLE_OPTION, PUBKEY_CONSOLE_OPTION, SIGFILE_CONSOLE_OPTION))
		{
			//do some code
		}
		else if (check_console_input_format(argv, ENCRYPT_CONSOLE_OPTION, INFILE_CONSOLE_OPTION, PUBKEY_CONSOLE_OPTION, OUTFILE_CONSOLE_OPTION))
		{
			//do some code
		}
		else if (check_console_input_format(argv, DECRYPT_CONSOLE_OPTION, INFILE_CONSOLE_OPTION, SECRET_CONSOLE_OPTION, OUTFILE_CONSOLE_OPTION))
		{
			//do some code
		}
		else
		{
			printf(ERROR_FIRST_PARAMETER, argv[1]);
			exit(WRONG_ARGUMENT_FAILURE);
		}
	}
	else
	{
		printf(ERROR_DEFAULT);
		exit(WRONG_ARGUMENT_FAILURE);
	}
	return 0;
}