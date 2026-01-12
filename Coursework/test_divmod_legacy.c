#include <stdio.h>
#include "lib/bigint/bit_LA.h"

int main() {
    // 10 / 3 = 3 remainder 1
    number a = int_to_number(10);
    number b = int_to_number(3);
    number ost = init();
    
    number q = division_with_module(&a, &b, &ost);
    
    printf("a: size=%d, values: ", a.current_count);
    for (int i = 0; i < a.current_count; i++) printf("%d ", a.mas[i]);
    printf("\n");
    printf("b: size=%d, values: ", b.current_count);
    for (int i = 0; i < b.current_count; i++) printf("%d ", b.mas[i]);
    printf("\n");
    printf("q: size=%d, values: ", q.current_count);
    for (int i = 0; i < q.current_count; i++) printf("%d ", q.mas[i]);
    printf("\n");
    printf("ost: size=%d, values: ", ost.current_count);
    for (int i = 0; i < ost.current_count; i++) printf("%d ", ost.mas[i]);
    printf("\n");
    
    clear_mem(&a);
    clear_mem(&b);
    clear_mem(&q);
    clear_mem(&ost);
    
    return 0;
}
