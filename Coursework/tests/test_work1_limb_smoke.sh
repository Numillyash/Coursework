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
check_out="$tmp_dir/check.out"
wrong_pub="$tmp_dir/wrong_public.key"

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

cat > "$wrong_pub" <<'KEY'
RSA_LIMB_KEY_V1
type=public
n=11cc3
e=11
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

expect_check_result() {
    local label="$1"
    local expected="$2"
    local out_file="$3"
    shift 3

    expect_success "$label" "$out_file" "$@"
    if ! grep -q "$expected" "$out_file"; then
        echo "FAIL: $label did not print: $expected" >&2
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

sign_check_roundtrip() {
    local label="$1"
    local plain="$2"
    local signature="$tmp_dir/$label.signature"
    local cmd_out="$tmp_dir/$label.sign.out"

    expect_success "$label sign" "$cmd_out" \
        ./work1_limb sign --infile "$plain" --secret "$sec" --sigfile "$signature"

    grep -q '^RSA_LIMB_BLOCKS_V1$' "$signature"
    grep -q '^kind=signature$' "$signature"
    grep -q '^original_size=' "$signature"
    grep -q '^block_size=' "$signature"
    grep -q '^block_count=' "$signature"
    grep -q '^encoding=hex-be$' "$signature"
    tail -n 1 "$signature" | grep -q '^&$'

    expect_check_result "$label check" "File signature is correct!" "$cmd_out" \
        ./work1_limb check --infile "$plain" --pubkey "$pub" --sigfile "$signature"
}

./work1_limb --help > "$help_out"
grep -q "work1_limb encrypt" "$help_out"
grep -q "work1_limb sign" "$help_out"
./work1_limb -h > "$help_out"
grep -q "work1_limb decrypt" "$help_out"
grep -q "work1_limb check" "$help_out"

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

sign_check_roundtrip "empty" "$empty"
sign_check_roundtrip "one" "$one"
sign_check_roundtrip "text" "$text"
sign_check_roundtrip "zeros" "$zeros"
sign_check_roundtrip "leading_zeros" "$leading_zeros"
sign_check_roundtrip "all_bytes" "$all_bytes"
sign_check_roundtrip "partial" "$partial"

modified_input="$tmp_dir/modified_input.bin"
printf "B" > "$modified_input"
expect_check_result "modified input is semantic invalid" \
    "File signature is NOT correct!" "$check_out" \
    ./work1_limb check --infile "$modified_input" --pubkey "$pub" \
        --sigfile "$tmp_dir/one.signature"

modified_signature="$tmp_dir/modified.signature"
sed '0,/^\*c\*.*#$/s//\*c\*0#/' \
    "$tmp_dir/one.signature" > "$modified_signature"
expect_check_result "modified signature is semantic invalid" \
    "File signature is NOT correct!" "$check_out" \
    ./work1_limb check --infile "$one" --pubkey "$pub" \
        --sigfile "$modified_signature"

expect_check_result "wrong public key is semantic invalid" \
    "File signature is NOT correct!" "$check_out" \
    ./work1_limb check --infile "$one" --pubkey "$wrong_pub" \
        --sigfile "$tmp_dir/one.signature"

# Appending/removing a block without updating block_count is malformed native
# format, so the strict parser rejects it with a nonzero execution error.
appended_signature="$tmp_dir/appended.signature"
awk '/^&$/ { print "*c*0#" } { print }' \
    "$tmp_dir/one.signature" > "$appended_signature"
expect_fail "appended signature block violates block_count" "$invalid_out" \
    ./work1_limb check --infile "$one" --pubkey "$pub" \
        --sigfile "$appended_signature"

removed_signature="$tmp_dir/removed.signature"
awk 'BEGIN { removed=0 } /^\*c\*/ && !removed { removed=1; next } { print }' \
    "$tmp_dir/one.signature" > "$removed_signature"
expect_fail "removed signature block violates block_count" "$invalid_out" \
    ./work1_limb check --infile "$one" --pubkey "$pub" \
        --sigfile "$removed_signature"

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
expect_fail "sign missing infile" "$missing_out" \
    ./work1_limb sign --infile "$tmp_dir/no_such_input.bin" --secret "$sec" --sigfile "$tmp_dir/bad.signature"
expect_fail "sign missing secret" "$missing_out" \
    ./work1_limb sign --infile "$one" --secret "$tmp_dir/no_such_secret.key" --sigfile "$tmp_dir/bad.signature"
expect_fail "sign missing sigfile argument" "$missing_out" \
    ./work1_limb sign --infile "$one" --secret "$sec"
expect_fail "check missing infile" "$missing_out" \
    ./work1_limb check --infile "$tmp_dir/no_such_input.bin" --pubkey "$pub" --sigfile "$tmp_dir/one.signature"
expect_fail "check missing pubkey" "$missing_out" \
    ./work1_limb check --infile "$one" --pubkey "$tmp_dir/no_such_public.key" --sigfile "$tmp_dir/one.signature"
expect_fail "check missing sigfile" "$missing_out" \
    ./work1_limb check --infile "$one" --pubkey "$pub" --sigfile "$tmp_dir/no_such.signature"
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
cat > "$tmp_dir/invalid_signature.txt" <<'BLOCKS'
RSA_LIMB_BLOCKS_V1
kind=signature
original_size=1
block_size=2
block_count=1
encoding=hex-be
*c*z#
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
expect_fail "invalid private key rejected by sign" "$invalid_out" \
    ./work1_limb sign --infile "$one" --secret "$tmp_dir/invalid_private.key" --sigfile "$tmp_dir/bad.signature"
expect_fail "invalid public key rejected by check" "$invalid_out" \
    ./work1_limb check --infile "$one" --pubkey "$tmp_dir/invalid_public.key" --sigfile "$tmp_dir/one.signature"
expect_fail "invalid signature file" "$invalid_out" \
    ./work1_limb check --infile "$one" --pubkey "$pub" --sigfile "$tmp_dir/invalid_signature.txt"
expect_fail "cipher-kind blocks rejected by check" "$invalid_out" \
    ./work1_limb check --infile "$one" --pubkey "$pub" --sigfile "$tmp_dir/one.cipher"
expect_fail "public key file rejected by sign" "$invalid_out" \
    ./work1_limb sign --infile "$one" --secret "$pub" --sigfile "$tmp_dir/bad.signature"
expect_fail "secret key file rejected by check" "$invalid_out" \
    ./work1_limb check --infile "$one" --pubkey "$sec" --sigfile "$tmp_dir/one.signature"

echo "work1_limb smoke test PASSED"
