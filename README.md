# RSA Coursework

This repository contains an RSA coursework implementation with two command-line
backends:

- `work1`: the original legacy C backend. This remains the stable/oracle
  implementation.
- `work1_tc`: a separate opt-in C++ backend using the transliteration-first
  `BitBigIntTC` BigInt port.

Both binaries support the same RSA command shapes and use compatible key,
ciphertext, and signature file formats. The legacy C backend remains the oracle
for BigInt parity tests and RSA cross-backend smoke tests.

## Build

```sh
make MODE=debug all
make MODE=debug work1_tc
make MODE=debug test
```

`make MODE=debug all` builds the legacy `work1` binary and benchmark binary.
`work1_tc` is intentionally opt-in and is not part of `all`.

## Legacy CLI: work1

```sh
./work1 genkey  --size <bits> --pubkey <pub.txt> --secret <sec.txt>
./work1 encrypt --infile <in.txt> --pubkey <pub.txt> --outfile <out.txt>
./work1 decrypt --infile <in.txt> --secret <sec.txt> --outfile <out.txt>
./work1 sign    --infile <in.txt> --secret <sec.txt> --sigfile <sig.txt>
./work1 check   --infile <in.txt> --pubkey <pub.txt> --sigfile <sig.txt>
```

Supported key sizes are `256`, `512`, `1024`, and `2048`.

## BitBigIntTC CLI: work1_tc

```sh
./work1_tc genkey  --size <bits> --pubkey <pub.txt> --secret <sec.txt>
./work1_tc encrypt --infile <in.txt> --pubkey <pub.txt> --outfile <out.txt>
./work1_tc decrypt --infile <in.txt> --secret <sec.txt> --outfile <out.txt>
./work1_tc sign    --infile <in.txt> --secret <sec.txt> --sigfile <sig.txt>
./work1_tc check   --infile <in.txt> --pubkey <pub.txt> --sigfile <sig.txt>
```

`work1_tc` is experimental compatibility-stage tooling. It is useful for
cross-checking the C++ `BitBigIntTC` port against the legacy file formats and
legacy RSA behavior. It is not a replacement for `work1` yet.

## Tests

Recommended normal checks:

```sh
make MODE=debug test
make MODE=debug test-divmod-stress
bash -x ./test_all.sh
```

Current test coverage:

- `test_tc_vs_legacy`: compares `BitBigIntTC` arithmetic against the legacy C
  `bit_LA.c` oracle.
- `test-rsa-tc-smoke`: tests reusable `RSA_TC` helpers and cross-backend file
  format compatibility.
- `test-work1-tc-smoke`: tests the `work1_tc` CLI against legacy `work1` on
  small cross-backend cases.
- `test_all.sh`: legacy `work1` end-to-end keygen, encrypt, decrypt, sign, and
  check coverage.

`make MODE=debug test` runs the normal C++ tests plus the TC RSA smoke tests.
`test_all.sh` is still legacy-focused.

## Caveats

- `BitBigIntTC` is a transliteration-first compatibility port of the legacy
  bit-level BigInt implementation. The representation has intentionally not
  been redesigned.
- Several `BitBigIntTC` methods are currently named `*_compat_for_testing`.
  They are compatibility APIs used by `RSA_TC` while behavior parity is being
  established.
- TC-backed 256-bit modular exponentiation may be slow before any optimization
  work.
- Performance notes below are historical coursework notes for the original
  implementation and should not be read as claims about `work1_tc`.

## Historical Notes

Оценка оптимизаций на разных этапах производилась путем сравнение  свободных клетотаблиц времени к ключу,
затраченного на зашифаровку, расшифровку файла.
## Время работы алгоритма без оптимизации
![image](https://user-images.githubusercontent.com/60771708/213115475-275a2153-fa5a-4b1d-b201-bd233bb8b210.png)

На этом и последующих рисунках будет показываться размер ключа в битах, время в секундах, минутах и часах, в первом, втором, третьем и четвертом столбце соответсветнно. Кроме того, 
в последующих столбцах приводятся сравнения по затраченному времени относительно других результатов 
(времени без оптимизаций (эталон) или промежуточных результатов).
## Оптимизация на уровне алгоритма
Для данной оптимизации был выбран алгоритм перемножения многочленов методом быстрого преобразования Фурье 
Для перемножения чисел в исходной программе используется метод Карацубы, сложность алгоритма которого: O(n^(log_2(3)))

![image](https://user-images.githubusercontent.com/60771708/213115826-d85d1c1f-1539-4502-9dc0-370d65b2176f.png)

## Машинно-независимая оптимизация 
В машинно-независимых оптимизациях были свернуты некоторые циклы, развернут короткий цикл и заменен цикл for на внутреннию функцию языка Си (memcpy)

![image](https://user-images.githubusercontent.com/60771708/213116671-c953356e-c3e7-4f6c-b993-de891411b254.png)

## Ассемлерная вставка 
Время выполнения программы с aсселблерной вставкой незначительно уменьшилось.

![image](https://user-images.githubusercontent.com/60771708/213116854-45b1f804-e08c-4391-bc44-19faa44ff633.png)

## Машино-зависимая оптимизация

Время выполнения программы с машинно-зависимой оптимизацией незначительно увеличилось.

![image](https://user-images.githubusercontent.com/60771708/213117305-bdbbeeb9-370f-4e1d-a5da-6012656d05e0.png)

## Финальное время выполнения
Финальное время выполнения алгоритма суммарно уменьшилось в 4 раза.

![image](https://user-images.githubusercontent.com/60771708/213117514-932f3197-87ec-4e5e-ad84-4ce4ce9f487c.png)



