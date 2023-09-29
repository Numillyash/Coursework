#include "_log_.h"

char date[18];
char filename[29];

void _log_start()
{
	FILE *file;
	const time_t time_programm_started = time(NULL);
	struct tm *u = localtime(&time_programm_started);
#ifdef __linux__
	mkdir("logs", S_IRWXU);
#elif _WIN32
	mkdir("logs");
#endif

	strftime(date, 18, "%H.%M.%S %d.%m.%y", u);

	snprintf(filename, 29, "./logs/%s.log", date);
	printf("%s\n", filename);
	file = fopen(filename, "w");
	if (file == NULL)
	{
		exit(LOG_FAILURE);
	}
	else
	{
		fprintf(file, "%s: CREATED LOG\n", date);
	}
	fclose(file);
}

void _log(char *message)
{
	FILE *file;
	const time_t time_programm_started = time(NULL);
	struct tm *u = localtime(&time_programm_started);
	strftime(date, 18, "%H.%M.%S %d.%m.%y", u);

	file = fopen(filename, "a");
	if (file == NULL)
	{
		exit(LOG_FAILURE);
	}
	else
	{
		fprintf(file, "%s: %s\n", date, message);
	}
	fclose(file);
}

void _log_ull(unsigned long long value)
{
	char message[40];
	snprintf(message, 30, "%llu", value);
	_log(message);
}