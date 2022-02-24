#include "config.h"
#include "help.h"

#include "longArithmetic.h"

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
					exit(WRONG_ARGUMENT);
				}
			}
			else
			{
				printf(WRONG_PARAMETER, argv[4], arg_2);
				exit(WRONG_ARGUMENT);
			}
		}
		else
		{
			printf(WRONG_PARAMETER, argv[2], arg_1);
			exit(WRONG_ARGUMENT);
		}
	}
	else
		return 0;
}


int main(int argc, char* argv[])
{

	number n1 = int_to_number(1e6);
	n1 = proizv(&n1, &n1); //12
	n1 = proizv(&n1, &n1); //24
	n1 = proizv(&n1, &n1); //48
	n1 = proizv(&n1, &n1); //96
	n1 = proizv(&n1, &n1); //192
	n1 = proizv(&n1, &n1); //384
	n1 = proizv(&n1, &n1); //768
	print_number(&n1);

	number n2 = int_to_number(67);
	number n3 = init();

	number rem, quot = deli_v_stolbik(&n1, &n2, &rem);
	//231200

	print_number(&quot);
	print_number(&rem);
	

	if (argc == 1)
	{
		printf(ERROR_NO_PARAMETERS);
		exit(NO_ARGUMENTS);
	}
	else if (argc == 2)
	{
		if (!strcmp(argv[1], HELP_CONSOLE_OPTION_1) || !strcmp(argv[1], HELP_CONSOLE_OPTION_2))
		{
			printf(HELP_TEXT);
			exit(WRONG_ARGUMENT);
		}
		else if (strcmp(argv[1], GENKEY_CONSOLE_OPTION) && strcmp(argv[1], CHECK_CONSOLE_OPTION) && strcmp(argv[1], SIGNATURE_CONSOLE_OPTION) && strcmp(argv[1], ENCRYPT_CONSOLE_OPTION) && strcmp(argv[1], DECRYPT_CONSOLE_OPTION))
		{
			printf(ERROR_FIRST_PARAMETER, argv[1]);
			exit(WRONG_ARGUMENT);
		}
		else 
		{
			printf(ERROR_NOT_ENOUGH_PARAMETERS);
			exit(NOT_ENOUGH_ARGUMENTS);
		}
	}
	else if (argc == 8)
	{
		if (     check_console_input_format(argv, GENKEY_CONSOLE_OPTION,    SIZE_CONSOLE_OPTION,   PUBKEY_CONSOLE_OPTION, SECRET_CONSOLE_OPTION ))
		{
			//do some code
		}
		else if (check_console_input_format(argv, SIGNATURE_CONSOLE_OPTION, INFILE_CONSOLE_OPTION, SECRET_CONSOLE_OPTION, SIGFILE_CONSOLE_OPTION))
		{
			//do some code
		}
		else if (check_console_input_format(argv, CHECK_CONSOLE_OPTION,     INFILE_CONSOLE_OPTION, PUBKEY_CONSOLE_OPTION, SIGFILE_CONSOLE_OPTION))
		{
			//do some code
		}
		else if (check_console_input_format(argv, ENCRYPT_CONSOLE_OPTION,   INFILE_CONSOLE_OPTION, PUBKEY_CONSOLE_OPTION, OUTFILE_CONSOLE_OPTION))
		{
			//do some code
		}
		else if (check_console_input_format(argv, DECRYPT_CONSOLE_OPTION,   INFILE_CONSOLE_OPTION, SECRET_CONSOLE_OPTION, OUTFILE_CONSOLE_OPTION))
		{
			//do some code
		}
		else
		{
			printf(ERROR_FIRST_PARAMETER, argv[1]);
			exit(WRONG_ARGUMENT);
		}
	}
	else
	{
		printf(ERROR_DEFAULT, argv[1]);
		exit(WRONG_ARGUMENT);
	}
}