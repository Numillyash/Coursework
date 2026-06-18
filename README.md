# RSA Coursework

This repository contains an RSA coursework implementation with three backend
layers:

- `work1`: the original legacy C backend. This remains the stable/oracle
  implementation.
- `work1_tc`: a separate opt-in C++ backend using the transliteration-first
  `BitBigIntTC` BigInt port.
- `BigUint` / `RSA_Limb`: a new experimental unsigned limb backend and raw RSA
  layer exposed through the separate opt-in `work1_limb` CLI.

The `work1` and `work1_tc` binaries support the same RSA command shapes and use
compatible key, ciphertext, and signature file formats. The legacy C backend
remains the oracle for BigInt parity tests and RSA cross-backend smoke tests.
`work1_limb` uses its own native formats and is not file-compatible with
`work1` or `work1_tc`.

## Build

```sh
make MODE=debug all
make MODE=debug work1_tc
make MODE=debug work1_limb
make MODE=debug test
```

`make MODE=debug all` builds the legacy `work1` binary and benchmark binary.
`work1_tc` and `work1_limb` are intentionally opt-in and are not part of `all`.

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

## BigUint CLI: work1_limb

```sh
./work1_limb -h
./work1_limb --help
./work1_limb encrypt --infile <input.bin> --pubkey <public.key> --outfile <cipher.txt>
./work1_limb decrypt --infile <cipher.txt> --secret <secret.key> --outfile <output.bin>
./work1_limb sign    --infile <input.bin> --secret <secret.key> --sigfile <signature.txt>
./work1_limb check   --infile <input.bin> --pubkey <public.key> --sigfile <signature.txt>
```

For `check`, a valid signature prints `File signature is correct!` and exits
with status 0. A semantic mismatch prints `File signature is NOT correct!` and
also exits with status 0. Parser, I/O, key-type, and execution errors return a
nonzero status.

`work1_limb` uses native RSA_Limb files:

- Keys start with `RSA_LIMB_KEY_V1` and contain lowercase big-endian hex values.
- Cipher and signature files start with `RSA_LIMB_BLOCKS_V1`.
- Block metadata includes `original_size`, `block_size`, `block_count`, and
  `encoding=hex-be`.
- Each encrypted or signature value is stored as `*c*<hex>#`.

These native files are deliberately distinct from the legacy `_n_`, `_e_`,
`_d_`, and `_c_` formats used by `work1` and `work1_tc`.

## BigUint / RSA_Limb

`BigUint` is the next experimental arithmetic backend. It is separate from the
legacy bit-level implementation and from `BitBigIntTC`.

Current `BigUint` representation and scope:

- `uint32_t` limbs with `uint64_t` double-limb arithmetic.
- Little-endian limb order: limb 0 stores the least significant 32 bits.
- Unsigned-only semantics.
- Canonical zero is an empty limb vector.
- Nonzero values are normalized with no leading zero limbs.
- Current arithmetic includes construction, comparison, bit operations, shifts,
  addition, absolute subtraction, schoolbook multiplication, square, divmod,
  div/mod wrappers, gcd, modular add/sub/mul, modular exponentiation, and
  modular inverse.

`RSA_Limb` is a minimal raw RSA layer on top of `BigUint`. It currently supports:

- keypair construction from supplied primes;
- public/private RSA operations through `BigUint::mod_pow`;
- raw byte encrypt/decrypt helpers;
- raw sign/check helpers.

`RSA_Limb` now has the opt-in native-format `work1_limb` CLI, but it does not
provide legacy key/cipher/signature compatibility, prime-file loading, genkey,
padding, hashing, PKCS#1, OAEP, or PSS. Its encryption and signing operations
are textbook/raw RSA test helpers only; they are not production cryptographic
schemes.

## Tests

Recommended normal checks:

```sh
make MODE=debug test
bash -x ./test_all.sh
```

Pre-release or deeper validation:

```sh
make MODE=debug test-divmod-stress
make MODE=debug test-tc-heavy
make MODE=asan test-sanitize
```

Current test coverage:

- `test_biguint`: tests the `BigUint` limb backend, including deterministic
  limb/`uint64_t` cases and selected positive-value comparisons against the
  `BitBigIntTC` oracle.
- `test_tc_vs_legacy`: compares `BitBigIntTC` arithmetic against the legacy C
  `bit_LA.c` oracle.
- `test-tc-heavy`: opt-in deterministic heavy `BitBigIntTC` parity corpus with
  broader randomized, boundary, threshold, module-power, and Euclid coverage.
- `test-rsa-tc-smoke`: tests reusable `RSA_TC` helpers and cross-backend file
  format compatibility.
- `test-rsa-limb-smoke`: tests `RSA_Limb` key construction, raw RSA arithmetic,
  raw byte encrypt/decrypt roundtrips, and raw sign/check helpers.
- `test-work1-tc-smoke`: tests the `work1_tc` CLI against legacy `work1` on
  small cross-backend cases, including CLI negative cases and deterministic
  RSA_TC parser edge cases.
- `test-work1-limb-smoke`: tests native `work1_limb` encrypt/decrypt/sign/check
  flows, binary payloads and zero bytes, strict native-format parser failures,
  and semantic signature failures.
- `test-divmod-stress`: opt-in diagnostic stress coverage for `BitBigIntTC`
  division/modulo.
- `test-sanitize`: opt-in AddressSanitizer/UndefinedBehaviorSanitizer subset
  for BigIntTC parity and TC RSA smoke tests. LeakSanitizer is disabled with
  `ASAN_OPTIONS=detect_leaks=0` in this environment.
- `test_all.sh`: legacy `work1` end-to-end keygen, encrypt, decrypt, sign, and
  check coverage.

`make MODE=debug test` runs the normal C++ tests, including `test_biguint`,
`test-rsa-limb-smoke`, `test-rsa-limb-format`, `test-work1-limb-smoke`, and the
TC RSA smoke tests. `test_all.sh` is still legacy-focused.

## Caveats

- `BitBigIntTC` is a transliteration-first compatibility port of the legacy
  bit-level BigInt implementation. The representation has intentionally not
  been redesigned.
- Several `BitBigIntTC` methods are currently named `*_compat_for_testing`.
  They are compatibility APIs used by `RSA_TC` while behavior parity is being
  established.
- TC-backed 256-bit modular exponentiation may be slow before any optimization
  work.
- `RSA_Limb` raw byte/signature helpers are textbook/raw RSA test helpers only.
  `work1_limb` does not provide padding, hashing, PKCS#1, OAEP, or PSS and
  should not be treated as production cryptography.
- Performance notes below are historical coursework notes for the original
  implementation and should not be read as claims about `work1_tc`.

## Current Status / Next Work

Current branch status:

- `work1` remains the unchanged legacy C backend and oracle.
- `work1_tc` remains a separate compatibility/reference backend for the
  transliteration-first `BitBigIntTC` implementation.
- The native `work1_limb` encrypt/decrypt/sign/check CLI milestone is complete.
  It remains experimental, raw/textbook RSA and uses formats that are not
  compatible with `work1` or `work1_tc`.

Likely next work:

- add native `work1_limb` genkey support;
- add an explicit legacy-compatible mode if required;
- performance work such as Knuth division, Montgomery reduction, or Barrett
  reduction;
- research padding, hashing, and signature schemes before any production-style
  cryptographic interface; none are implemented yet.

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
