#include "_file_.h"

char date[18];
char filename[29];

void _log_start()
{
	FILE* file;
	const time_t time_programm_started = time(NULL);
	struct tm* u = localtime(&time_programm_started);
#ifdef __linux__ 
	mkdir("logs", S_IRWXU);
#elif _WIN32
	mkdir("logs");
#else

#endif
	strftime(date, 18, "%H.%M.%S %d.%m.%y", u);
	strcat(filename, "./logs/");
	strcat(filename, date);
	strcat(filename, ".log");
	printf(filename); printf("\n");
	file = fopen(filename, "w");
	if (file == NULL)
	{
		exit(LOG_FAILURE);
	}
	else
	{
		fputs(date, file);
		fputs(": ", file);
		fputs("CREATED LOG", file);
		fputs("\n", file);
	}
	fclose(file);
}

void _log(char* message)
{
	FILE* file;
	const time_t time_programm_started = time(NULL);
	struct tm* u = localtime(&time_programm_started);
	strftime(date, 18, "%H.%M.%S %d.%m.%y", u);
	
	file = fopen(filename, "a");
	if (file == NULL)
	{
		exit(LOG_FAILURE);
	}
	else
	{
		fputs(date, file);
		fputs(": ", file);
		fputs(message, file);
		fputs("\n", file);
	}
	fclose(file);
	
}

number get_prime_from_file(char* filename, int line_number, int bit_size) 
{

}