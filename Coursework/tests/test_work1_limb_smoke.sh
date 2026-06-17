#!/usr/bin/env bash
set -euo pipefail

tmp_dir="test_results/tmp/work1_limb_smoke"
mkdir -p "$tmp_dir"

pub="$tmp_dir/public.key"
sec="$tmp_dir/secret.key"
help_out="$tmp_dir/help.out"
arg_out="$tmp_dir/arg.out"
missing_out="$tmp_dir/missing.out"
invalid_out="$tmp_dir/invalid.out"

cat > "$pub" <<'KEY'
RSA_LIMB_KEY_V1
type=public
n=10807
e=11
&
KEY

cat > "$sec" <<'KEY'
RSA_LIMB_KEY_V1
type=secret
n=10807
d=b8f1
&
KEY

expect_fail() {
    local label="$1"
    local out_file="$2"
    shift 2

    if "$@" > "$out_file" 2>&1; then
        echo "FAIL: expected command to fail: $label" >&2
        cat "$out_file" >&2
        exit 1
    fi
}

expect_success() {
    local label="$1"
    local out_file="$2"
    shift 2

    if ! "$@" > "$out_file" 2>&1; then
        echo "FAIL: expected command to succeed: $label" >&2
        cat "$out_file" >&2
        exit 1
    fi
}

roundtrip() {
    local label="$1"
    local plain="$2"
    local cipher="$tmp_dir/$label.cipher"
    local out="$tmp_dir/$label.out"
    local cmd_out="$tmp_dir/$label.cmd.out"

    expect_success "$label encrypt" "$cmd_out" \
        ./work1_limb encrypt --infile "$plain" --pubkey "$pub" --outfile "$cipher"

    grep -q '^RSA_LIMB_BLOCKS_V1$' "$cipher"
    grep -q '^kind=cipher$' "$cipher"
    grep -q '^original_size=' "$cipher"
    grep -q '^block_size=' "$cipher"
    grep -q '^encoding=hex-be$' "$cipher"
    tail -n 1 "$cipher" | grep -q '^&$'

    expect_success "$label decrypt" "$cmd_out" \
        ./work1_limb decrypt --infile "$cipher" --secret "$sec" --outfile "$out"
    cmp -s "$plain" "$out"
}

./work1_limb --help > "$help_out"
grep -q "work1_limb encrypt" "$help_out"
./work1_limb -h > "$help_out"
grep -q "work1_limb decrypt" "$help_out"

empty="$tmp_dir/empty.bin"
one="$tmp_dir/one.bin"
text="$tmp_dir/text.bin"
zeros="$tmp_dir/zeros.bin"
leading_zeros="$tmp_dir/leading_zeros.bin"
all_bytes="$tmp_dir/all_bytes.bin"
partial="$tmp_dir/partial.bin"

: > "$empty"
printf "A" > "$one"
printf "hello limb rsa" > "$text"
printf '\000\001\002\000\377' > "$zeros"
printf '\000\000A\001' > "$leading_zeros"
for i in $(seq 0 255); do
    printf "\\$(printf '%03o' "$i")"
done > "$all_bytes"
printf '\001\002\003\004\005' > "$partial"

roundtrip "empty" "$empty"
roundtrip "one" "$one"
roundtrip "text" "$text"
roundtrip "zeros" "$zeros"
roundtrip "leading_zeros" "$leading_zeros"
roundtrip "all_bytes" "$all_bytes"
roundtrip "partial" "$partial"

expect_fail "unknown command" "$arg_out" ./work1_limb unknown
expect_fail "wrong option name" "$arg_out" \
    ./work1_limb encrypt --bad "$one" --pubkey "$pub" --outfile "$tmp_dir/bad.cipher"
expect_fail "too few args" "$arg_out" ./work1_limb encrypt
expect_fail "too many args" "$arg_out" \
    ./work1_limb encrypt --infile "$one" --pubkey "$pub" --outfile "$tmp_dir/bad.cipher" extra

expect_fail "missing infile" "$missing_out" \
    ./work1_limb encrypt --infile "$tmp_dir/no_such_input.bin" --pubkey "$pub" --outfile "$tmp_dir/bad.cipher"
expect_fail "missing pubkey" "$missing_out" \
    ./work1_limb encrypt --infile "$one" --pubkey "$tmp_dir/no_such_public.key" --outfile "$tmp_dir/bad.cipher"
expect_fail "missing secret" "$missing_out" \
    ./work1_limb decrypt --infile "$tmp_dir/one.cipher" --secret "$tmp_dir/no_such_secret.key" --outfile "$tmp_dir/bad.out"
expect_fail "missing outfile parent" "$missing_out" \
    ./work1_limb encrypt --infile "$one" --pubkey "$pub" --outfile "$tmp_dir/no_such_dir/bad.cipher"

cat > "$tmp_dir/invalid_public.key" <<'KEY'
RSA_LIMB_KEY_V1
type=public
n=0
e=11
&
KEY
cat > "$tmp_dir/invalid_private.key" <<'KEY'
RSA_LIMB_KEY_V1
type=secret
n=10807
d=0
&
KEY
cat > "$tmp_dir/invalid_cipher.txt" <<'BLOCKS'
RSA_LIMB_BLOCKS_V1
kind=cipher
original_size=1
block_size=2
block_count=1
encoding=hex-be
*c*z#
&
BLOCKS
cat > "$tmp_dir/signature_kind.txt" <<'BLOCKS'
RSA_LIMB_BLOCKS_V1
kind=signature
original_size=0
block_size=2
block_count=0
encoding=hex-be
&
BLOCKS

expect_fail "invalid public key" "$invalid_out" \
    ./work1_limb encrypt --infile "$one" --pubkey "$tmp_dir/invalid_public.key" --outfile "$tmp_dir/bad.cipher"
expect_fail "invalid private key" "$invalid_out" \
    ./work1_limb decrypt --infile "$tmp_dir/empty.cipher" --secret "$tmp_dir/invalid_private.key" --outfile "$tmp_dir/bad.out"
expect_fail "invalid cipher file" "$invalid_out" \
    ./work1_limb decrypt --infile "$tmp_dir/invalid_cipher.txt" --secret "$sec" --outfile "$tmp_dir/bad.out"
expect_fail "signature-kind blocks rejected by decrypt" "$invalid_out" \
    ./work1_limb decrypt --infile "$tmp_dir/signature_kind.txt" --secret "$sec" --outfile "$tmp_dir/bad.out"

echo "work1_limb smoke test PASSED"
