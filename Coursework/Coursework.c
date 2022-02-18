#include "config.h"
#include "help.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		printf(ERROR_NO_PARAMETERS);
	}
	else if (argc == 2)
	{
		if (!strcmp(argv[1], HELP_CONSOLE_OPTION_1) || !strcmp(argv[1], HELP_CONSOLE_OPTION_2))
			printf(HELP_TEXT);
		else if (strcmp(argv[1], GENKEY_CONSOLE_OPTION) && strcmp(argv[1], CHECK_CONSOLE_OPTION) && strcmp(argv[1], SIGNATURE_CONSOLE_OPTION) && strcmp(argv[1], ENCRYPT_CONSOLE_OPTION) && strcmp(argv[1], DECRYPT_CONSOLE_OPTION))
			printf(ERROR_PARAMETER, argv[1]);
		else
			printf(ERROR_NOT_ENOUGH_PARAMETERS);
	}
	else if (argc == 8)
	{
		if (!strcmp(argv[1], GENKEY_CONSOLE_OPTION))
		{

		}
		else if (!strcmp(argv[1], CHECK_CONSOLE_OPTION))
		{

		}
		else if (!strcmp(argv[1], SIGNATURE_CONSOLE_OPTION))
		{

		}
		else if (!strcmp(argv[1], ENCRYPT_CONSOLE_OPTION))
		{

		}
		else if (!strcmp(argv[1], DECRYPT_CONSOLE_OPTION))
		{

		}
		else
		{
			printf(ERROR_PARAMETER, argv[1]);
		}
	}
}