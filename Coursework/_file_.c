#include "_file_.h"

char filename[24];

void _log(char* message)
{
	FILE* file;

#ifdef IS_LOG_CREATED
	const time_t time_programm_started = time(NULL);
	struct tm* u = localtime(&time_programm_started);
	strftime(filename, 24, "%H:%M:%S %x.log", u);
	printf(filename);
	file = fopen(filename, "w");
	if (file == NULL)
	{
		exit(LOG_FAILURE);
	}
	else
	{
		fputs("log created", file);
	}
	fclose(file);
	#undef IS_LOG_CREATED
#endif 

	file = fopen(filename, "a");
	if (file == NULL)
	{
		exit(LOG_FAILURE);
	}
	else
	{
		fputs(message, file);
	}
	fclose(file);
	
}

number get_prime_from_file(char* filename, int line_number, int bit_size) 
{

}