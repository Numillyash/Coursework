# RSA Coursework agent instructions

This project contains a legacy C bit-level BigInt implementation and a C++ transliteration port.

Main rule:
We are intentionally doing a transliteration-first port from legacy C to C++.
Do NOT redesign the BigInt representation yet.

Priorities:
1. Preserve legacy behavior exactly.
2. Use lib/bigint/bit_LA.c as the oracle.
3. Compare C++ BitBigIntTC against legacy C on identical inputs.
4. Do not introduce uint64_t limb representation yet.
5. Do not rewrite RSA.c unless a dedicated failing test proves a bug.
6. Do not optimize before equivalence tests are green.
7. Do not mix behavior changes and refactoring in one patch.
8. Prefer small, reviewable patches.
9. Every change must be justified by tests.
10. Always run relevant tests before final response.

Important files:
- lib/bigint/bit_LA.c
- lib/bigint/bit_LA.h
- lib/bigint/BitBigIntTC.cpp
- lib/bigint/BitBigIntTC.hpp
- tests/test_tc_vs_legacy.cpp
- tests/legacy_bridge.hpp
- Makefile
- test_all.sh
- tests/*.sh

Expected workflow:
- inspect first
- summarize current state
- propose patch plan
- patch only after understanding current structure
- run tests from console
- report changed files and exact commands
