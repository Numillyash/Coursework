# RSA / BIGINT Architecture Canvas

**Проект:** собственная производительная BigInt/RSA-библиотека  
**Цель:** сделать библиотеку своими руками, с полным пониманием внутренней арифметики, пригодную для дальнейшей оптимизации и портирования, в том числе на микроконтроллеры.  
**Текущий контекст:** старый учебный bit-level bigint остаётся oracle/reference; `BitBigIntTC` завершил compatibility/reference фазу; ветка `LimbBigUint-core` уже содержит первый рабочий `BigUint` limb-backend и минимальный `RSA_Limb` raw RSA слой для тестов.

---

## 0. Главная философия проекта

Библиотека должна быть не просто «работающей RSA-реализацией», а собственной низкоуровневой арифметической библиотекой.

Ключевые принципы:

1. **Полный контроль над представлением числа.**
   Никаких внешних big-int библиотек в ядре.
2. **Понимание каждого уровня:**
   от бита и machine word до Montgomery, CRT и RSA.
3. **Переносимость.**
   Код должен потенциально переноситься на микроконтроллеры.
4. **Производительность.**
   Архитектура должна позволять ускорять операции без полного переписывания API.
5. **Учебная ценность.**
   Старый bit-level bigint не выбрасывается, а используется как reference backend / oracle для проверки нового limb-backend.

---

## 0.1. Текущий статус ветки LimbBigUint-core

Сейчас в проекте существуют три слоя backend'ов:

1. **Legacy C backend `work1`.**
   Использует `lib/bigint/bit_LA.c` и `lib/rsa/RSA.c`.
   Это стабильная исходная реализация и oracle для проверки поведения.
2. **Compatibility/reference backend `work1_tc`.**
   Использует transliteration-first C++ порт `BitBigIntTC` и совместимые legacy
   форматы ключей, шифртекста и подписи.
3. **Experimental limb backend `BigUint` / `RSA_Limb`.**
   Это новый unsigned limb-backend и raw RSA test layer. У него пока нет CLI,
   legacy file-format интеграции, чтения prime-файлов или padding/signature
   scheme уровня PKCS/PSS.

Текущий `BigUint`:

- хранит число в `uint32_t` limb'ах;
- использует little-endian порядок limb'ов;
- является unsigned-only типом;
- представляет zero как пустой `std::vector` limb'ов;
- поддерживает canonical invariant: у ненулевых чисел нет leading zero limb'ов;
- уже имеет construction, compare, bit_length, test_bit/set_bit, shifts, add,
  `sub_abs`, schoolbook multiplication, square, divmod/div/mod, gcd,
  `mod_add`, `mod_sub`, `mod_mul`, `mod_pow`, `mod_inverse`.

Текущий `RSA_Limb`:

- строит keypair из переданных простых `p`, `q` и экспоненты `e`;
- выполняет public/private operation через `BigUint::mod_pow`;
- содержит raw byte encrypt/decrypt helpers;
- содержит raw sign/check helpers;
- предназначен для тестов raw/textbook RSA, а не для production cryptography;
- не реализует padding, hashing, PKCS#1, OAEP или PSS.

Тестовый статус:

- `make MODE=debug test` включает `test_biguint` и `test-rsa-limb-smoke`;
- `test_biguint` проверяет `BigUint` на deterministic fixtures и выбранных
  positive-value oracle-сравнениях с `BitBigIntTC`;
- `test-rsa-limb-smoke` проверяет keypair construction, raw RSA operations,
  raw byte encrypt/decrypt roundtrips и raw sign/check helpers;
- дополнительные проверки остаются отдельными: `test-tc-heavy`,
  `test-sanitize`, `test_all.sh`.

Ближайшая работа:

1. Спроектировать RSA_Limb serialization и возможный отдельный CLI.
2. Добавить performance-слой: Knuth division, Montgomery reduction, Barrett
   reduction.
3. При необходимости сделать legacy-compatible режим для форматов старого RSA.

---

## 1. ADR-001 — Базовое представление числа

### Решение

Основной производительный backend должен хранить число как массив limb'ов:

```cpp
using limb_t = uint32_t;
using dlimb_t = uint64_t;
```

Хранение:

```cpp
std::vector<uint32_t> limbs_; // little-endian
```

Математически:

```text
X = limbs[0] + limbs[1] * B + limbs[2] * B^2 + ...
B = 2^32
```

То есть `limbs[0]` — младшие 32 бита, `limbs[n-1]` — старшие.

### Почему `uint32_t`, а не битовый массив

Битовый массив хорош для обучения, но плох для производительности:

- 1024-битное число в bit-backend = примерно 1024 элемента;
- 1024-битное число в `uint32_t` backend = 32 limb'а;
- процессор естественно работает со словами, а не с отдельными битами;
- `uint32_t * uint32_t` гарантированно помещается в `uint64_t`;
- это переносимо на desktop и многие MCU.

### Статус

Принято и реализовано в первой версии `BigUint`.

---

## 2. ADR-002 — Старый bit-level bigint

### Решение

Старую битовую реализацию не удалять.

Она остаётся как:

1. reference implementation;
2. oracle для property-тестов;
3. учебный вариант, где буквально видно каждый бит;
4. база для сравнения поведения нового limb-backend.

### Как использовать

Для случайных тестов:

```text
случайные a, b
операция в BitBigInt
операция в LimbBigUint
сравнение результата
```

Операции для сравнения:

- add;
- sub;
- compare;
- shift;
- mul;
- square;
- divmod;
- mod;
- powmod.

### Статус

Принято. На практике эту роль сейчас выполняют legacy `bit_LA.c` и
compatibility/reference backend `BitBigIntTC`.

---

## 3. ADR-003 — BigUint прежде BigInt

### Решение

Основное ядро делать как **беззнаковое** большое число:

```cpp
class BigUint;
```

RSA почти полностью работает с беззнаковыми значениями:

- `n`;
- `e`;
- `d`;
- `p`;
- `q`;
- `phi`;
- ciphertext;
- message;
- residues.

Signed `BigInt` нужен в основном для некоторых вариантов extended Euclid / mod inverse, но не должен усложнять основное ядро.

### Возможная структура

```cpp
class BigUint {
public:
    using Limb = uint32_t;
    static constexpr unsigned LIMB_BITS = 32;

private:
    std::vector<Limb> limbs_; // little-endian
};
```

Позже можно добавить:

```cpp
class BigInt {
    bool negative_;
    BigUint magnitude_;
};
```

### Статус

Принято и реализовано в `BigUint`. Публичный signed `BigInt` пока не добавлен.

---

## 4. ADR-004 — Число как positional limbs и как полином

### Важное понимание

Работать с набором блоков как с числом корректно.

`BigUint` — это число в позиционной системе счисления по основанию `B = 2^32`.

```text
X = x0 + x1*B + x2*B^2 + ...
```

Но для умножения полезно временно смотреть на него как на полином:

```text
X(t) = x0 + x1*t + x2*t^2 + ...
```

Тогда умножение — это polynomial convolution:

```text
r[k] = sum(a[i] * b[j]), где i + j = k
```

После этого нужна carry normalization, потому что коэффициенты могут быть больше `B - 1`.

### Вывод

Оба взгляда правильные:

1. **числовой взгляд** — для compare, divmod, mod, gcd;
2. **полиномиальный взгляд** — для mul, square, Karatsuba, Toom-Cook, FFT/NTT;
3. **машинный взгляд** — для limb operations, carry, wide multiply.

### Статус

Принято как базовая ментальная модель.

---

## 5. ADR-005 — Union для разложения uint64_t не использовать

### Идея

Была идея использовать `union`, чтобы результат `uint32_t * uint32_t` раскладывать на два `uint32_t`.

### Решение

Не использовать `union` для этого.

Вместо этого:

```cpp
uint64_t product = uint64_t(a) * uint64_t(b);

uint32_t lo = uint32_t(product);
uint32_t hi = uint32_t(product >> 32);
```

### Почему

`union` может зависеть от:

- endian;
- type-punning;
- особенностей ABI;
- компилятора.

Сдвиги и маски:

- переносимы;
- очевидны;
- хорошо оптимизируются компилятором.

### Статус

Принято.

---

## 6. ADR-006 — Low-level primitive layer

### Решение

Сделать низкоуровневый слой операций над limb'ами.

Пример:

```cpp
namespace low {

using limb_t = uint32_t;
using dlimb_t = uint64_t;

inline limb_t add_carry(limb_t a, limb_t b, limb_t carry_in, limb_t& carry_out) {
    dlimb_t sum = dlimb_t(a) + b + carry_in;
    carry_out = limb_t(sum >> 32);
    return limb_t(sum);
}

inline limb_t sub_borrow(limb_t a, limb_t b, limb_t borrow_in, limb_t& borrow_out) {
    dlimb_t bb = dlimb_t(b) + borrow_in;
    borrow_out = a < bb;
    return limb_t(dlimb_t(a) - bb);
}

inline void mul_wide(limb_t a, limb_t b, limb_t& lo, limb_t& hi) {
    dlimb_t p = dlimb_t(a) * b;
    lo = limb_t(p);
    hi = limb_t(p >> 32);
}

}
```

### Зачем

Верхний bigint не должен знать, как именно реализован carry/wide multiply.

Позже можно добавить платформенные оптимизации:

- x86 intrinsics;
- ARM intrinsics;
- inline asm;
- MCU-specific backend.

### Статус

Принято.

---

## 7. ADR-007 — ROTR / rotate-through-carry

### Вопрос

Есть ли в C/C++ операция вроде ROTR, которая при битовом сдвиге сохраняет старший/младший бит в регистр?

### Ответ

В C++20 есть:

```cpp
#include <bit>

std::rotl(x, k);
std::rotr(x, k);
```

Но это циклический rotate внутри одного слова.

Для bigint важнее не `rotl/rotr`, а:

- add with carry;
- sub with borrow;
- wide multiply;
- count leading zeroes;
- shift by arbitrary number of bits;
- Montgomery reduction.

Прямого переносимого доступа к CPU carry flag в стандартном C/C++ нет.

Portable сдвиг limb'ов делается явно:

```cpp
uint32_t carry = 0;

for (size_t i = 0; i < limbs.size(); ++i) {
    uint32_t new_carry = limbs[i] >> 31;
    limbs[i] = (limbs[i] << 1) | carry;
    carry = new_carry;
}
```

### Статус

ROTR не является приоритетом для bigint. Приоритет — carry-aware primitives.

---

## 8. ADR-008 — Сдвиги

### Решение

Не делать hot path через многократный `shift by 1`.

Нужны операции:

```cpp
shift_left_bits(size_t k);
shift_right_bits(size_t k);
```

Сдвиг на `k` бит:

```text
word_shift = k / 32
bit_shift  = k % 32
```

Делается одним проходом по limb'ам.

### Почему

Многократный сдвиг по одному биту на больших числах слишком дорогой.

### Статус

Принято.

---

## 9. ADR-009 — Умножение

### Базовый алгоритм

Сначала реализовать schoolbook multiplication на limb'ах.

```cpp
for (size_t i = 0; i < a.size(); ++i) {
    uint64_t carry = 0;

    for (size_t j = 0; j < b.size(); ++j) {
        uint64_t cur =
            uint64_t(result[i + j]) +
            uint64_t(a[i]) * uint64_t(b[j]) +
            carry;

        result[i + j] = uint32_t(cur);
        carry = cur >> 32;
    }

    // carry нужно аккуратно протащить дальше
}
```

### Следующие этапы

1. Schoolbook multiplication.
2. Separate square.
3. Comba multiplication.
4. Karatsuba.
5. Toom-Cook.
6. FFT/NTT — сильно позже, если вообще понадобится.

### Статус

Принято: сначала schoolbook + correct carry propagation.

---

## 10. ADR-010 — Square отдельно

### Решение

Заложить отдельный интерфейс:

```cpp
BigUint square(const BigUint& a);
```

На первом этапе он может вызывать:

```cpp
return mul(a, a);
```

Позже можно оптимизировать за счёт симметрии.

### Почему

В RSA modpow много операций вида:

```text
x = x * x mod n
```

Отдельный square может дать ускорение.

### Статус

Принято.

---

## 11. ADR-011 — Деление

### Решение

Нужно иметь общий:

```cpp
struct DivResult {
    BigUint quot;
    BigUint rem;
};

DivResult divmod(const BigUint& a, const BigUint& b);
```

### Этапы

1. Сначала можно сделать простой binary long division поверх limb-backend.
2. Потом перейти на Knuth Algorithm D по limb'ам.

### Важный принцип

В RSA hot path обычное деление должно почти исчезнуть.

В `modpow` нельзя постоянно делать:

```text
mul -> divmod -> remainder
```

Вместо этого нужен Montgomery.

### Статус

Принято.

---

## 12. ADR-012 — Modular arithmetic layer

### Решение

Не хранить все числа глобально в “чисто модульном виде”.

Вместо этого:

```cpp
class MontgomeryContext {
public:
    explicit MontgomeryContext(const BigUint& modulus);

    BigUint to_mont(const BigUint& x) const;
    BigUint from_mont(const BigUint& x) const;

    BigUint mul(const BigUint& a_mont, const BigUint& b_mont) const;
    BigUint square(const BigUint& a_mont) const;
    BigUint pow(const BigUint& base, const BigUint& exp) const;

private:
    BigUint n_;
    uint32_t n0_inv_;
    BigUint r2_mod_n_;
};
```

### Почему

RSA постоянно делает:

```text
(a * b) mod n
```

Обычный `%` через деление слишком дорогой.

Montgomery multiplication убирает обычное деление из hot path.

### Статус

Принято.

---

## 13. ADR-013 — RNS / CRT representation

### Решение

Не использовать RNS/CRT как основной формат хранения bigint.

RNS:

```text
X = [X mod m1, X mod m2, X mod m3, ...]
```

Плюсы:

- быстрое независимое сложение/умножение;
- хорошо параллелится.

Минусы:

- сложно сравнивать;
- сложно делить;
- сложно делать gcd;
- сложно делать mod inverse;
- дорого переводить туда/обратно;
- сложно контролировать диапазон.

### Вывод

RNS — не первый этап и не основной backend.

CRT использовать точечно в RSA private operation.

### Статус

Принято.

---

## 14. ADR-014 — RSA CRT private operation

### Решение

Private RSA делать через CRT.

Вместо:

```text
m = c^d mod n
```

делать:

```text
m1 = c^dp mod p
m2 = c^dq mod q
h  = qInv * (m1 - m2) mod p
m  = m2 + q * h
```

Где:

```text
dp = d mod (p - 1)
dq = d mod (q - 1)
qInv = q^(-1) mod p
```

### Структура ключа

```cpp
struct RsaPrivateKey {
    BigUint n;
    BigUint e;
    BigUint d;

    BigUint p;
    BigUint q;

    BigUint dp;
    BigUint dq;
    BigUint qInv;
};
```

### Статус

Принято.

---

## 15. ADR-015 — Экспонента в modpow

### Решение

Не мутировать exponent в процессе `modpow`.

Нужны методы:

```cpp
size_t bit_length() const;
bool test_bit(size_t i) const;
```

Простейший вариант:

```cpp
BigUint result = ctx.to_mont(BigUint::one());
BigUint x = ctx.to_mont(base % mod);

for (size_t i = 0; i < exp.bit_length(); ++i) {
    if (exp.test_bit(i)) {
        result = ctx.mul(result, x);
    }
    x = ctx.square(x);
}

return ctx.from_mont(result);
```

### Следующий этап

Sliding/fixed window exponentiation.

### Статус

Принято.

---

## 16. ADR-016 — Embedded / MCU portability

### Решение

Сразу предусмотреть два режима хранения.

### Desktop/dev mode

```cpp
class DynamicBigUint {
    std::vector<uint32_t> limbs_;
};
```

### Embedded mode

```cpp
template<size_t MaxLimbs>
class StaticBigUint {
    std::array<uint32_t, MaxLimbs> limbs_;
    size_t size_;
};
```

Для RSA-2048:

```text
2048 / 32 = 64 limbs
```

Но для промежуточных результатов умножения нужно до `2n` limb'ов.

### Статус

Предварительно принято.

---

## 17. Предварительный roadmap

### Stage 0 — Инфраструктура

- [x] Бенчмарк `bench_rsa`.
- [x] `perf stat`.
- [x] `perf record`.
- [x] `callgrind`.
- [ ] Единый формат отчётов по бенчам.
- [ ] Property tests между old bit backend и new limb backend.

### Stage 1 — Limb BigUint core

- [x] `BigUint` на `uint32_t`.
- [x] `normalize`.
- [x] `compare`.
- [x] `bit_length`.
- [x] `test_bit`.
- [x] `set_bit`.
- [x] `add`.
- [x] `sub_abs`.
- [x] `shift_left_bits`.
- [x] `shift_right_bits`.
- [x] `mul_schoolbook`.
- [x] `square`.

### Stage 2 — Division and number theory

- [x] `divmod`.
- [x] `mod`.
- [x] `gcd`.
- [x] `extended_gcd` или другой `mod_inverse`.
- [ ] Miller-Rabin primality test.
- [ ] Prime generation.

### Stage 3 — Modular arithmetic

- [ ] Barrett reduction, возможно как промежуточный вариант.
- [ ] MontgomeryContext.
- [ ] Montgomery multiplication.
- [ ] Montgomery square.
- [x] Basic `mod_add` / `mod_sub` / `mod_mul`.
- [x] Basic binary `mod_pow`.
- [ ] Montgomery pow.
- [ ] Sliding window exponentiation.

### Stage 4 — RSA

- [x] `RsaPublicKey` / `PublicKeyLimb`.
- [x] `RsaPrivateKey` / `PrivateKeyLimb`.
- [ ] key generation.
- [x] keypair from supplied primes.
- [x] public operation.
- [x] private operation slow.
- [ ] private operation CRT.
- [x] basic raw byte block encoding.
- [x] raw byte encrypt/decrypt helpers.
- [x] raw sign/check helpers.
- [ ] later: OAEP/PSS-like modes.

### Stage 5 — Optimization

- [ ] Comba multiplication.
- [ ] Optimized square.
- [ ] Karatsuba.
- [ ] platform low-level intrinsics.
- [ ] constant-time conditional operations.
- [ ] static-memory embedded backend.

---

## 18. Текущий главный вывод

Самая правильная архитектура для цели проекта:

```text
старый bit-bigint = reference backend
новый backend = uint32_t limbs, little-endian
арифметика = обычный BigUint + отдельные modular contexts
RSA private = CRT
modpow = Montgomery
MCU portability = static backend later
```

Не делать:

```text
глобальный RNS/CRT bigint
union-based limb splitting
bit-array as primary performance backend
heavy OOP hierarchy
```

Делать:

```text
limb primitives
clear invariants
separate BigUint core
separate MontgomeryContext
separate RSA layer
tests + perf after each stage
```

---

## 19. Альтернативные направления развития

Этот раздел хранит не только уже принятые решения, но и варианты, которые стоит держать в поле зрения. Часть из них может стать отдельными экспериментальными ветками.

---

## 19.1. Варианты представления числа

### Вариант A — bit-array backend

Число хранится как массив битов.

Плюсы:

- максимально наглядно;
- удобно для обучения;
- легко понять переносы, сдвиги, дополнительный код;
- уже есть рабочая реализация.

Минусы:

- очень низкая плотность хранения;
- много лишних операций;
- плохо использует машинные слова;
- сдвиги/умножение/деление слишком дорогие для производительной RSA.

Роль в проекте:

```text
reference backend / учебный backend / oracle для тестов
```

---

### Вариант B — `uint16_t` limbs

Основание:

```text
B = 2^16
```

Плюсы:

- хорошо для слабых 8/16-битных MCU;
- `uint16_t * uint16_t` помещается в `uint32_t`;
- легче портировать на старые микроконтроллеры.

Минусы:

- для desktop и 32-bit MCU обычно хуже `uint32_t`;
- больше limb'ов;
- больше циклов.

Роль:

```text
возможный embedded backend для слабых платформ
```

---

### Вариант C — `uint32_t` limbs

Основание:

```text
B = 2^32
```

Плюсы:

- хороший баланс переносимости и скорости;
- `uint32_t * uint32_t -> uint64_t`;
- подходит для desktop и большинства 32-bit MCU;
- не требует `__int128`.

Минусы:

- на 64-bit desktop может быть медленнее `uint64_t` backend;
- для очень слабых MCU `uint64_t` операции могут быть дорогими.

Роль:

```text
основной backend по умолчанию
```

---

### Вариант D — `uint64_t` limbs

Основание:

```text
B = 2^64
```

Плюсы:

- меньше limb'ов;
- быстрее на x86_64/AArch64;
- хорошо для desktop/server.

Минусы:

- для умножения нужен `unsigned __int128` или intrinsics;
- хуже переносимость;
- на MCU может быть сильно медленнее.

Роль:

```text
опциональный high-performance desktop backend
```

---

### Вариант E — fixed-size bigint

Число хранится в массиве фиксированного размера:

```cpp
template<size_t MaxLimbs>
class StaticBigUint;
```

Плюсы:

- нет heap allocation;
- предсказуемая память;
- удобно для MCU;
- проще делать constant-time операции.

Минусы:

- нужен контроль переполнения capacity;
- менее удобно для произвольной точности;
- сложнее API.

Роль:

```text
embedded / constant-time / RSA fixed-size backend
```

---

### Вариант F — dynamic bigint

Число хранится в `std::vector<limb_t>`.

Плюсы:

- удобно для разработки;
- произвольная длина;
- проще тесты и эксперименты.

Минусы:

- heap allocation;
- сложнее портировать на bare-metal MCU;
- аллокации могут испортить производительность.

Роль:

```text
desktop/dev backend
```

---

## 19.2. Варианты знакового представления

### Вариант A — unsigned-only core

Основное ядро — только `BigUint`.

Плюсы:

- проще инварианты;
- проще RSA;
- проще modular arithmetic;
- меньше багов.

Минусы:

- для extended GCD иногда нужен signed wrapper или аккуратная modular inverse логика.

Роль:

```text
основной подход
```

---

### Вариант B — sign-magnitude BigInt

```cpp
struct BigInt {
    bool negative;
    BigUint magnitude;
};
```

Плюсы:

- простая логика;
- удобно для extended GCD;
- не усложняет `BigUint`.

Минусы:

- нужно следить за `-0`;
- больше ветвлений.

Роль:

```text
опциональный wrapper
```

---

### Вариант C — two's complement bigint

Плюсы:

- ближе к машинной signed арифметике;
- некоторые битовые операции выглядят естественно.

Минусы:

- сильно усложняет arbitrary precision;
- нормализация знака сложнее;
- RSA почти не требует этого.

Роль:

```text
не использовать как основной формат
```

---

## 19.3. Варианты умножения

### Schoolbook multiplication

Базовое умножение:

```text
O(n^2)
```

Плюсы:

- просто;
- легко проверить;
- хорошо для малых и средних размеров;
- must-have fallback.

Минусы:

- не лучшее для очень больших чисел.

Роль:

```text
первый production-correct алгоритм
```

---

### Comba multiplication

Считает результат по диагоналям.

Плюсы:

- меньше обращений к памяти;
- хорошо для фиксированных размеров;
- полезно для RSA-1024/2048;
- удобно оптимизировать вручную.

Минусы:

- сложнее код;
- нужно аккуратно работать с аккумуляторами.

Роль:

```text
следующий шаг после schoolbook
```

---

### Separate square

Оптимизированное возведение в квадрат.

Плюсы:

- в modpow квадратов очень много;
- можно использовать симметрию;
- обычно проще Karatsuba.

Минусы:

- отдельный код и тесты.

Роль:

```text
сделать рано, но сначала square может вызывать mul(a,a)
```

---

### Karatsuba

```text
O(n^1.585)
```

Плюсы:

- хороший учебный и практический шаг;
- полезен для больших чисел;
- естественно ложится на полиномиальную модель.

Минусы:

- для малых размеров может быть медленнее schoolbook;
- нужен cutoff;
- много временных объектов.

Роль:

```text
после стабильного schoolbook + benchmark cutoff
```

---

### Toom-Cook

Плюсы:

- быстрее Karatsuba на больших размерах;
- логическое продолжение polynomial view.

Минусы:

- заметно сложнее;
- много edge cases;
- для RSA-2048 может быть не нужен.

Роль:

```text
дальний эксперимент
```

---

### FFT / NTT multiplication

Плюсы:

- хорошо для очень больших чисел;
- интересно теоретически.

Минусы:

- сложно;
- требует аккуратной работы с ошибками/модулями;
- для обычного RSA почти точно избыточно.

Роль:

```text
не первый и не средний этап
```

---

## 19.4. Варианты деления и редукции

### Binary long division

Плюсы:

- просто;
- легко реализовать;
- удобно как первый divmod.

Минусы:

- медленно;
- слишком много bit-level операций.

Роль:

```text
первый корректный divmod для тестов
```

---

### Knuth Algorithm D

Классическое деление по limb'ам.

Плюсы:

- нормальный общий divmod;
- хорошая учебная ценность;
- нужен для полноценного bigint.

Минусы:

- сложная нормализация;
- много edge cases;
- сложно отлаживать.

Роль:

```text
целевой общий divmod
```

---

### Burnikel-Ziegler division

Плюсы:

- лучше для очень больших чисел;
- блочное деление.

Минусы:

- сложнее Knuth;
- для RSA-2048 может быть избыточно.

Роль:

```text
дальний этап
```

---

### Barrett reduction

Плюсы:

- ускоряет `x mod m`, если модуль фиксирован;
- проще Montgomery;
- работает и для чётных модулей.

Минусы:

- для RSA Montgomery обычно лучше;
- нужны предвычисления.

Роль:

```text
возможный промежуточный modular context
```

---

### Montgomery reduction

Плюсы:

- лучший базовый выбор для RSA;
- убирает обычное деление из modpow;
- отлично работает для нечётных модулей.

Минусы:

- нужно Montgomery domain;
- сложнее обычного `%`;
- нужны предвычисления `n0_inv`, `R^2 mod n`.

Роль:

```text
основной RSA modular backend
```

---

## 19.5. Варианты modpow

### Binary exponentiation LSB-first

Плюсы:

- просто;
- легко реализовать;
- хорошо для первого Montgomery pow.

Минусы:

- не минимальное число умножений;
- branching по битам exponent.

Роль:

```text
первый корректный modpow
```

---

### Binary exponentiation MSB-first

Плюсы:

- естественно для left-to-right exponentiation;
- проще перейти к window methods.

Минусы:

- всё ещё много умножений.

Роль:

```text
следующий после LSB-first
```

---

### Fixed window exponentiation

Плюсы:

- меньше умножений;
- хорошо для RSA private/public;
- предсказуемый паттерн.

Минусы:

- нужно предвычисление таблицы;
- память;
- для constant-time нужна аккуратная выборка.

Роль:

```text
важный performance step
```

---

### Sliding window exponentiation

Плюсы:

- меньше умножений, чем binary;
- эффективен для больших exponent.

Минусы:

- более переменный паттерн;
- side-channel риски.

Роль:

```text
для speed backend, не для constant-time backend
```

---

### Addition chains

Плюсы:

- можно оптимизировать фиксированные exponent;
- особенно интересно для e=65537.

Минусы:

- не универсально;
- для private exponent сложнее.

Роль:

```text
special-case для public exponent
```

---

## 19.6. RSA-уровень

### Textbook RSA

Плюсы:

- просто;
- полезно для обучения.

Минусы:

- небезопасно;
- нельзя считать production scheme.

Роль:

```text
только учебный / internal primitive
```

---

### RSA public exponent special-case

Для `e = 65537`:

```text
65537 = 2^16 + 1
```

Можно делать:

```text
16 squarings + 1 multiply
```

Плюсы:

- резко быстрее общего modpow;
- очень просто.

Минусы:

- только для public operation.

Роль:

```text
обязательная оптимизация public RSA
```

---

### RSA private slow path

```text
m = c^d mod n
```

Плюсы:

- просто;
- нужно как reference.

Минусы:

- медленно.

Роль:

```text
reference/private fallback
```

---

### RSA private CRT

Плюсы:

- ускорение private operation примерно в несколько раз;
- стандартный подход.

Минусы:

- нужно хранить p/q/dp/dq/qInv;
- нужна защита от fault attacks в production-like режиме.

Роль:

```text
основной private RSA path
```

---

### Multi-prime RSA

```text
n = p*q*r*...
```

Плюсы:

- private operation ещё быстрее;
- интересная CRT-архитектура.

Минусы:

- сложнее ключи;
- хуже совместимость;
- больше security нюансов.

Роль:

```text
дальний эксперимент
```

---

## 19.7. Side-channel / security architecture

### Constant-time backend

Плюсы:

- ближе к реальной криптобиблиотеке;
- меньше timing/cache leaks.

Минусы:

- сложнее;
- иногда медленнее;
- требует дисциплины во всех слоях.

Роль:

```text
отдельный backend или режим
```

---

### Blinding

Для RSA private operation можно использовать blinding.

Плюсы:

- защита от timing/power analysis;
- классическая защита RSA.

Минусы:

- нужен RNG;
- дополнительная стоимость.

Роль:

```text
production-like режим
```

---

### CRT fault check

После CRT private operation можно проверить:

```text
result^e mod n == ciphertext
```

Плюсы:

- защита от fault attacks;
- полезно для CRT RSA.

Минусы:

- дополнительная public operation.

Роль:

```text
production-like режим
```

---

## 19.8. API / архитектурные варианты

### Header-only

Плюсы:

- удобно для templates;
- легко подключать.

Минусы:

- долгие сборки;
- сложнее ABI;
- много кода в заголовках.

Роль:

```text
не основной вариант
```

---

### Static library

Плюсы:

- просто;
- удобно для embedded;
- хороший контроль сборки.

Минусы:

- меньше гибкости ABI.

Роль:

```text
основной формат поставки
```

---

### C API поверх C++ ядра

Плюсы:

- удобно портировать;
- можно вызывать из C;
- стабильный ABI;
- проще FFI.

Минусы:

- нужно аккуратно проектировать ownership.

Роль:

```text
желательно после стабилизации C++ core
```

---

### Backend policy / templates

Например:

```cpp
template<class Storage, class LowLevelOps>
class BasicBigUint;
```

Плюсы:

- можно иметь Dynamic/Static backend;
- можно иметь 16/32/64-bit limbs;
- удобно для embedded.

Минусы:

- сложнее код;
- больше compile-time шума.

Роль:

```text
после простой реализации BigUint
```

---

## 19.9. Память и аллокации

### Heap-based

Плюсы:

- просто;
- удобно для разработки.

Минусы:

- плохо для MCU;
- может шуметь в профилях;
- не constant-time по allocation behavior.

Роль:

```text
dev/backend по умолчанию
```

---

### Arena allocator

Плюсы:

- меньше malloc/free;
- удобно для временных чисел;
- лучше контроль памяти.

Минусы:

- нужно проектировать lifetime;
- можно словить сложные bugs.

Роль:

```text
важный performance/embedded вариант
```

---

### Stack/static temporaries

Плюсы:

- хорошо для фиксированных RSA размеров;
- без heap;
- быстро.

Минусы:

- ограничение размера;
- API сложнее.

Роль:

```text
embedded/fixed RSA backend
```

---

## 19.10. Тестовая архитектура

### Deterministic tests

- фиксированные seed;
- одинаковые входы между backend;
- JSONL benchmark output.

### Property tests

Для случайных `a`, `b`:

```text
new_backend(op(a,b)) == reference_backend(op(a,b))
```

### Differential tests

Сравнение с внешней библиотекой в dev-only режиме:

- Python `int`;
- OpenSSL BN;
- GMP.

Внешние библиотеки не входят в core, но могут использоваться как тестовый oracle.

### Fuzzing

- операции bigint;
- парсинг/сериализация;
- RSA block decode;
- edge cases.

### Benchmark gates

После каждой архитектурной оптимизации:

```text
before/after perf stat
before/after callgrind
before/after correctness tests
```

---

## 20. Обновлённый список перспективных направлений

Короткий список того, что можно изучать/реализовывать дальше:

1. Limb backend `uint32_t`.
2. Static backend для MCU.
3. Low-level primitive abstraction.
4. Schoolbook multiplication.
5. Comba multiplication.
6. Separate square.
7. Knuth Algorithm D division.
8. BarrettContext.
9. MontgomeryContext.
10. Fixed/sliding window modpow.
11. Special-case `e = 65537`.
12. RSA CRT.
13. RSA blinding.
14. CRT fault verification.
15. Arena allocator.
16. Constant-time backend.
17. C API поверх C++ core.
18. Property tests против старого bit backend.
19. Differential tests против Python/GMP/OpenSSL.
20. Embedded profile без heap.

---

## 21. ADR-017 — Конфигурируемый размер limb

### Вопрос

Нужно ли сделать размер limb через `#define` / compile-time config, чтобы библиотека заводилась на микроконтроллерах?

### Решение

Да, размер limb должен быть конфигурируемым, но не произвольным числом бит.

Поддерживаемые профили:

```text
RSA_LIMB_BITS = 8
RSA_LIMB_BITS = 15
RSA_LIMB_BITS = 16
RSA_LIMB_BITS = 31
RSA_LIMB_BITS = 32
RSA_LIMB_BITS = 64
```

Основные production-профили:

```text
default portable: 32-bit limbs
small MCU:        15-bit или 16-bit limbs
desktop fast:     64-bit limbs, если есть корректный wide multiply
constant-time:    15/31-bit limbs как вариант BearSSL-like
```

### Почему не стоит делать любой произвольный `RSA_LIMB_BITS`

Алгоритмы завязаны на:

- тип limb;
- тип double-limb;
- переносы;
- маски;
- Montgomery `n0_inv`;
- count-leading-zeroes;
- сериализацию;
- поведение умножения.

Произвольное значение вроде 23 или 27 усложнит код без большой практической пользы.

### Рекомендуемая конфигурация

```cpp
#ifndef RSA_LIMB_BITS
#define RSA_LIMB_BITS 32
#endif

#if RSA_LIMB_BITS == 16
using limb_t = uint16_t;
using dlimb_t = uint32_t;
#elif RSA_LIMB_BITS == 32
using limb_t = uint32_t;
using dlimb_t = uint64_t;
#elif RSA_LIMB_BITS == 64
using limb_t = uint64_t;
// dlimb_t требует __int128 или platform intrinsic
#else
#error "Unsupported RSA_LIMB_BITS"
#endif
```

### Отдельная идея: payload bits vs storage bits

BearSSL-подобный подход:

```text
storage type: uint32_t
payload bits: 31
```

То есть limb хранится в 32-битном слове, но используется только 31 бит.

Плюсы:

- проще constant-time carry;
- есть запасной верхний бит;
- иногда быстрее и переносимее, чем полное использование 32 бит.

Аналогично:

```text
uint16_t storage, 15 payload bits
```

Это важно для слабых MCU и constant-time backend.

### Вывод

Ввести не просто `LIMB_BITS`, а две сущности:

```cpp
using limb_t = ...;
using dlimb_t = ...;

static constexpr unsigned STORAGE_BITS = sizeof(limb_t) * 8;
static constexpr unsigned PAYLOAD_BITS = RSA_LIMB_BITS;
static constexpr limb_t LIMB_MASK = (limb_t(1) << PAYLOAD_BITS) - 1;
```

Для `PAYLOAD_BITS == STORAGE_BITS` маска должна обрабатываться отдельно, чтобы не делать UB-сдвиг на ширину типа.

### Статус

Принято как направление.

Первый рабочий backend:

```text
limb_t = uint32_t
PAYLOAD_BITS = 32
dlimb_t = uint64_t
```

Потом добавить:

```text
uint16_t / 15-bit profile
uint32_t / 31-bit profile
uint64_t / 64-bit desktop profile
```

---

## 22. Обзор альтернативных реализаций bigint/RSA

Этот раздел фиксирует реальные архитектурные подходы, найденные в зрелых библиотеках.

---

### 22.1. GMP / GNU MP

Модель:

```text
low-level mpn layer
pointer to least significant limb + limb count
caller allocates destination
minimal overhead
```

Идея GMP: высокоуровневый `mpz` построен поверх очень низкоуровневого `mpn`. `mpn`-функции не пытаются быть удобным объектным API; они заточены под скорость и требуют, чтобы вызывающий код сам обеспечил память, размеры и корректность.

Что взять в наш дизайн:

1. Разделить высокий и низкий уровни.
2. Сделать внутренние функции вида:

```cpp
limb_t add_n(limb_t* dst, const limb_t* a, const limb_t* b, size_t n);
limb_t sub_n(limb_t* dst, const limb_t* a, const limb_t* b, size_t n);
void mul_n(limb_t* dst, const limb_t* a, const limb_t* b, size_t n);
```

3. Для performance-path избегать лишних проверок и аллокаций.
4. Высокий `BigUint` должен быть удобным, но внутри должны быть “голые” low-level kernels.

---

### 22.2. BearSSL

Модель:

```text
несколько реализаций: i15, i31, i32, i62
нет dynamic allocation
caller/storage allocated
constant-time by design
```

Ключевая идея BearSSL:

- i15: для ARM Cortex M0+/M3 и похожих слабых платформ;
- i31: generic backend, использует 31 payload bit в `uint32_t`;
- i32: исторический 32-bit backend, но i31 считается лучше;
- i62: вариант для 64→128 multiplication в modular exponentiation.

Что взять в наш дизайн:

1. Разделить storage bits и payload bits.
2. Рассмотреть `31-bit limbs in uint32_t`.
3. Поддержать no-heap режим.
4. Для cryptographic private operations помнить о constant-time.
5. В embedded-профиле предпочесть self-contained array representation.

---

### 22.3. mbedTLS MPI

Модель:

```text
mbedtls_mpi_uint = machine digit / limb
platform-dependent limb size
init/free/grow/shrink API
```

mbedTLS делает MPI как C-структуру с явным управлением памятью и функциями `init/free/grow/shrink`. Тип limb зависит от платформенной конфигурации.

Что взять:

1. Явный lifecycle для C API.
2. `grow()` не должен перевыделять память, если capacity уже достаточна.
3. Ошибки возвращаются статус-кодами, а не исключениями.
4. Для C API полезен стиль:

```c
rsa_mpi_init(&x);
rsa_mpi_grow(&x, nlimbs);
rsa_mpi_free(&x);
```

---

### 22.4. LibTomMath

Модель:

```text
portable C multiple-precision library
Comba multiplication
Montgomery / Barrett / DR reductions
Karatsuba / Toom-Cook
number theory functions
```

LibTomMath — хороший пример “учебно-производственной” библиотеки: portable C, много комментариев, широкий набор алгоритмов.

Что взять:

1. Несколько reduction algorithms:
   - Barrett;
   - Montgomery;
   - special reductions.
2. Comba multiplier/square как practical optimization.
3. Karatsuba/Toom-Cook с cutoff.
4. Широкий набор number theory:
   - gcd;
   - lcm;
   - modular inverse;
   - Miller-Rabin;
   - prime generation.

---

### 22.5. TomsFastMath

Модель:

```text
portable C
optional inline asm multipliers
no heap usage
optimized for speed
```

Что взять:

1. Отдельный fast backend.
2. No-heap profile.
3. Platform-specific multiply kernels.
4. Возможность добавлять asm/intrinsics без изменения высокого API.

---

### 22.6. OpenSSL BN

Модель:

```text
BIGNUM + BN_CTX temporary arena
BN_MONT_CTX for Montgomery
constant-time modular exponentiation with fixed windows
```

Что взять:

1. Отдельный context для временных чисел.
2. Отдельный Montgomery context.
3. Специальный const-time modexp path.
4. Fixed-window exponentiation для секретных exponent.

Идея для нашей библиотеки:

```cpp
class BigWorkspace;       // временные буферы
class MontgomeryContext;  // modulus-specific precomputation
```

---

### 22.7. Botan BigInt

Модель:

```text
signed magnitude BigInt
word = uint32_t или uint64_t depending on processor
внутри есть быстрые и side-channel safe arithmetic facilities
```

Что взять:

1. `BigInt` как signed-magnitude wrapper поверх unsigned magnitude.
2. Не раскрывать опасные низкоуровневые крипто-примитивы как “просто посчитай сам”.
3. Разделить public API и internal crypto-safe arithmetic.

---

### 22.8. micro-ecc / nano-ecc style

Модель embedded crypto:

```text
маленький код
фиксированные размеры
8/16/32-bit targets
минимальная память
часто нет общего bigint API
```

Что взять:

1. Для MCU лучше иметь specialized fixed-size backend.
2. Не всегда нужен arbitrary precision на embedded.
3. Для RSA fixed key size можно сделать `StaticBigUint<MaxLimbs>`.
4. Отдельный профиль сборки без heap.

---

## 23. Новые архитектурные выводы после обзора

### 23.1. Нужны минимум два уровня API

```text
high-level BigUint API
low-level limb kernel API
```

High-level:

```cpp
BigUint a, b;
BigUint c = a * b;
```

Low-level:

```cpp
limb_t add_n(limb_t* dst, const limb_t* a, const limb_t* b, size_t n);
limb_t sub_n(limb_t* dst, const limb_t* a, const limb_t* b, size_t n);
void mul_comba(limb_t* dst, const limb_t* a, const limb_t* b, size_t n);
```

### 23.2. `DEFINE` должен выбирать backend profile, а не просто число

Лучше:

```cpp
#define RSA_BACKEND_LIMB32
#define RSA_BACKEND_LIMB31
#define RSA_BACKEND_LIMB15
#define RSA_BACKEND_LIMB64
#define RSA_BACKEND_STATIC
#define RSA_BACKEND_DYNAMIC
```

А не только:

```cpp
#define LIMB_SIZE 32
```

Потому что 31-bit и 32-bit backend — это не просто разный размер. У них разные правила carry, mask, header/length и Montgomery.

### 23.3. Workspace нужен рано

Для производительности и embedded лучше не плодить временные `BigUint`.

Нужен объект:

```cpp
class BigWorkspace {
public:
    limb_t* alloc_limbs(size_t n);
    void reset();
};
```

Или C-style:

```c
rsa_ws_alloc(ws, nlimbs);
rsa_ws_reset(ws);
```

### 23.4. Dynamic и Static storage должны иметь общий algorithm layer

Идеальная цель:

```cpp
template<class Storage, class LimbPolicy>
class BasicBigUint;
```

Но не делать это первым шагом.

Сначала:

```text
DynamicBigUint<uint32_t, 32 payload bits>
```

Потом вынести общее.

### 23.5. Constant-time backend — отдельная архитектурная цель

Не надо случайно смешивать:

```text
fast bigint
crypto-safe private RSA bigint
```

Для начала делаем fast/educational backend, но API должен позволять позже добавить const-time private RSA path.
