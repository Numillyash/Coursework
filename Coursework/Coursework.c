#include "RSA.h"

int main(int argc, char *argv[])
{
	setlocale(LC_ALL, "rus");
	_log_start();

	int _a = -10;
	int _b = -10;
	int _c = -10;

	number A = int_to_number(_a);
	number B = int_to_number(_b);
	number C;

	printf("========\n");
	for (_a = -10; _a <= 10; _a++)
		for (_b = -10; _b <= 10; _b++)
		{
			clear_mem(&A);
			clear_mem(&B);
			A = int_to_number(_a);
			B = int_to_number(_b);
			C = multiplication(&A, &B);
			print_number_decimal(&C);
			printf("%d * %d = %d\n", _a, _b, _a * _b);
			//  print_number(&A);
			//  print_number_decimal(&A);
			//  printf("========\n");
		}
	// A = int_to_number(10);
	// additional_code(&A);
	// B = int_to_number(10);
	// C = multiplication(&A, &B);
	// printf("_c is %d\n", -100);
	// print_number(&C);
	// print_number_decimal(&C);
	// // print_number(&A);
	// // print_number(&B);
	return 1;
}