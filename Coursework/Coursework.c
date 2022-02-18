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
        if (strcmp(argv[1], HELP_CONSOLE_OPTION_1) == 0 || strcmp(argv[1], HELP_CONSOLE_OPTION_2) == 0)
            printf(HELP_TEXT);
        else
            printf(ERROR_FIRST_PARAMETER, argv[1]);
    }
}