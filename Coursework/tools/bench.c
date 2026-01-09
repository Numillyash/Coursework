#define _POSIX_C_SOURCE 200112L
#include "../lib/bigint/bit_LA.h"
#include "../include/config.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static uint64_t splitmix64_next(uint64_t *state)
{
    uint64_t z = (*state += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

static uint64_t now_ns()
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (uint64_t)t.tv_sec * 1000000000ULL + (uint64_t)t.tv_nsec;
}

static number rand_number_bits(uint64_t *st, int bits, int force_msb1, int force_odd)
{
    number x = init();
    for (int i = 0; i < bits; i++)
    {
        uint8_t bit = (uint8_t)(splitmix64_next(st) & 1ULL);
        if (i == bits - 1 && force_msb1)
            bit = 1;
        add_digit(&x, bit);
    }
    if (force_odd)
    {
        if (x.current_count > 0)
            x.mas[0] = 1;
    }
    normalize(&x);
    return x;
}

int main(int argc, char **argv)
{
    int keybits = 1024;
    int seeds = 10;
    int iters = 30;
    int warmup = 3;
    int print_inputs = 0;

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--keybits") == 0 && i + 1 < argc)
            keybits = atoi(argv[++i]);
        else if (strcmp(argv[i], "--seeds") == 0 && i + 1 < argc)
            seeds = atoi(argv[++i]);
        else if (strcmp(argv[i], "--iters") == 0 && i + 1 < argc)
            iters = atoi(argv[++i]);
        else if (strcmp(argv[i], "--warmup") == 0 && i + 1 < argc)
            warmup = atoi(argv[++i]);
        else if (strcmp(argv[i], "--print-inputs") == 0)
            print_inputs = 1;
    }

    uint64_t st = 0xfeedbeefcafebabeULL;

    for (int s = 0; s < seeds; s++)
    {
        // derive per-seed state deterministically
        st += (uint64_t)s;

        number n = rand_number_bits(&st, keybits, 1, 1);
        number a = rand_number_bits(&st, keybits - 2, 1, 0);
        number e = int_to_number(65537);
        number d_like = rand_number_bits(&st, keybits - 1, 1, 1);

        if (print_inputs)
        {
            printf("{\"seed\":%d,\"keybits\":%d}\n", s, keybits);
        }

        // warmup
        for (int w = 0; w < warmup; w++)
        {
            number tmp = module_pow(&a, &e, &n);
            clear_mem(&tmp);
            number tmp2 = module_pow(&a, &d_like, &n);
            clear_mem(&tmp2);
        }

        // public exp
        uint64_t start = now_ns();
        for (int it = 0; it < iters; it++)
        {
            number r = module_pow(&a, &e, &n);
            clear_mem(&r);
        }
        uint64_t end = now_ns();
        printf("{\"test\":\"modexp_public\",\"keybits\":%d,\"seed\":%d,\"iters\":%d,\"warmup\":%d,\"ns\":%llu}\n",
               keybits, s, iters, warmup, (unsigned long long)(end - start));

        // private-like exp
        start = now_ns();
        for (int it = 0; it < iters; it++)
        {
            number r = module_pow(&a, &d_like, &n);
            clear_mem(&r);
        }
        end = now_ns();
        printf("{\"test\":\"modexp_private_like\",\"keybits\":%d,\"seed\":%d,\"iters\":%d,\"warmup\":%d,\"ns\":%llu}\n",
               keybits, s, iters, warmup, (unsigned long long)(end - start));

        clear_mem(&n);
        clear_mem(&a);
        clear_mem(&e);
        clear_mem(&d_like);
    }

    return 0;
}
