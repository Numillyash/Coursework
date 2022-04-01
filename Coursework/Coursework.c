#include "RSA.h"

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
	// обойтись без хэша
	// все проверки на дурака

	setlocale(LC_ALL, "rus");
	_log_start();

	//genkey --size 2048 --pubkey pb.txt --secret sk.txt

	// fwrite();

	if (argc == 1)
	{
		printf(ERROR_NO_PARAMETERS);
		_log("User typed no parameters");
		exit(NO_ARGUMENTS_FAILURE);
	}
	else if (argc == 2)
	{
		if (!strcmp(argv[1], HELP_CONSOLE_OPTION_1) || !strcmp(argv[1], HELP_CONSOLE_OPTION_2))
		{
			printf(HELP_TEXT);
			_log("User typed -h or --help parameter");
			exit(WRONG_ARGUMENT_FAILURE);
		}
		else if (strcmp(argv[1], GENKEY_CONSOLE_OPTION) && strcmp(argv[1], CHECK_CONSOLE_OPTION) && strcmp(argv[1], SIGNATURE_CONSOLE_OPTION) && strcmp(argv[1], ENCRYPT_CONSOLE_OPTION) && strcmp(argv[1], DECRYPT_CONSOLE_OPTION))
		{
			printf(ERROR_FIRST_PARAMETER, argv[1]);
			_log("User typed wrong 1 parameter");
			exit(WRONG_ARGUMENT_FAILURE);
		}
		else
		{
			printf(ERROR_NOT_ENOUGH_PARAMETERS);
			_log("User typed not enough parameters");
			exit(NOT_ENOUGH_ARGUMENTS_FAILURE);
		}
	}
	else if (argc == 8)
	{
		// progname workmode arg1  --    arg2  --    arg3  --
		// argv0    argv1    argv2 argv3 argv4 argv5 argv6 argv7
		if (check_console_input_format     (argv, GENKEY_CONSOLE_OPTION,    SIZE_CONSOLE_OPTION,   PUBKEY_CONSOLE_OPTION, SECRET_CONSOLE_OPTION))
		{
			_log("User chose generate key option");
			check_filenames_2(argv[5], argv[7]);
			generate_key(argv[3], argv[5], argv[7]);
		}
		else if (check_console_input_format(argv, SIGNATURE_CONSOLE_OPTION, INFILE_CONSOLE_OPTION, SECRET_CONSOLE_OPTION, SIGFILE_CONSOLE_OPTION))
		{
			_log("User chose signify file option");
			check_filenames_3(argv[3], argv[5], argv[7]);
			signify_file(argv[3], argv[5], argv[7]);
		}
		else if (check_console_input_format(argv, CHECK_CONSOLE_OPTION,     INFILE_CONSOLE_OPTION, PUBKEY_CONSOLE_OPTION, SIGFILE_CONSOLE_OPTION))
		{
			_log("User chose check files sign option");
			check_filenames_3(argv[3], argv[5], argv[7]);
			check_sign_file(argv[3], argv[5], argv[7]);
		}
		else if (check_console_input_format(argv, ENCRYPT_CONSOLE_OPTION,   INFILE_CONSOLE_OPTION, PUBKEY_CONSOLE_OPTION, OUTFILE_CONSOLE_OPTION))
		{
			_log("User chose to encrypt file option");
			check_filenames_3(argv[3], argv[5], argv[7]);
			encrypt_file(argv[3], argv[5], argv[7]);
		}
		else if (check_console_input_format(argv, DECRYPT_CONSOLE_OPTION,   INFILE_CONSOLE_OPTION, SECRET_CONSOLE_OPTION, OUTFILE_CONSOLE_OPTION))
		{
			_log("User chose to decrypt file option");
			check_filenames_3(argv[3], argv[5], argv[7]);
			decrypt_file(argv[3], argv[5], argv[7]);
		}
		else
		{
			printf(ERROR_FIRST_PARAMETER, argv[1]);
			_log("User typed wrong parameters");
			exit(WRONG_ARGUMENT_FAILURE);
		}
	}
	else
	{
		printf(ERROR_DEFAULT);
		_log("User typed wrong number of parameters");
		exit(WRONG_ARGUMENT_FAILURE);
	}
	return 0;
}