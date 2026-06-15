#!/usr/bin/env bash
set -euo pipefail

tmp_dir="test_results/tmp/work1_tc_smoke"
key_dir="test_results/keys"

mkdir -p "$tmp_dir" "$key_dir"

small_pub="$tmp_dir/small_pb.txt"
small_sec="$tmp_dir/small_sc.txt"
plain="$tmp_dir/plain.txt"
tampered_plain="$tmp_dir/tampered_plain.txt"
empty_plain="$tmp_dir/empty.txt"
empty_blocks="$tmp_dir/empty_blocks.txt"
generated_pub="$key_dir/work1_tc_pb256.txt"
generated_sec="$key_dir/work1_tc_sc256.txt"
tc_cipher="$tmp_dir/tc_cipher.txt"
legacy_decrypted="$tmp_dir/legacy_decrypted.txt"
legacy_cipher="$tmp_dir/legacy_cipher.txt"
tc_decrypted="$tmp_dir/tc_decrypted.txt"
tc_sig="$tmp_dir/tc_signature.txt"
legacy_sig="$tmp_dir/legacy_signature.txt"
bad_sig="$tmp_dir/bad_signature.txt"
legacy_check_out="$tmp_dir/legacy_check.out"
tc_check_out="$tmp_dir/tc_check.out"
tc_check_tampered_out="$tmp_dir/tc_check_tampered.out"
tc_check_bad_sig_out="$tmp_dir/tc_check_bad_sig.out"
tc_empty_decrypted="$tmp_dir/tc_empty_decrypted.txt"
tc_empty_check_out="$tmp_dir/tc_empty_check.out"
legacy_empty_decrypted="$tmp_dir/legacy_empty_decrypted.txt"
legacy_empty_check_out="$tmp_dir/legacy_empty_check.out"
help_out="$tmp_dir/help.out"
legacy_help_out="$tmp_dir/legacy_help.out"
missing_out="$tmp_dir/missing.out"
arg_out="$tmp_dir/arg.out"
corrupt_out="$tmp_dir/corrupt.out"
wrong_pub="$tmp_dir/wrong_pub.txt"
appended_plain="$tmp_dir/appended_plain.txt"
legacy_check_tampered_out="$tmp_dir/legacy_check_tampered.out"
legacy_check_bad_sig_out="$tmp_dir/legacy_check_bad_sig.out"
legacy_check_appended_out="$tmp_dir/legacy_check_appended.out"
tc_check_appended_out="$tmp_dir/tc_check_appended.out"
tc_check_wrong_pub_out="$tmp_dir/tc_check_wrong_pub.out"
legacy_check_wrong_pub_out="$tmp_dir/legacy_check_wrong_pub.out"
tc_check_modified_sig_out="$tmp_dir/tc_check_modified_sig.out"
legacy_check_modified_sig_out="$tmp_dir/legacy_check_modified_sig.out"
tc_decrypt_trailing_out="$tmp_dir/tc_decrypt_trailing.txt"
legacy_decrypt_trailing_out="$tmp_dir/legacy_decrypt_trailing.txt"

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

cat > "$small_pub" <<'KEY'
_n_bkm#
_e_bb#
&
KEY

cat > "$small_sec" <<'KEY'
_n_bkm#
_d_bmk#
&
KEY

cat > "$wrong_pub" <<'KEY'
_n_hd#
_e_d#
&
KEY

printf "A" > "$plain"
printf "B" > "$tampered_plain"
printf "Atail" > "$appended_plain"
: > "$empty_plain"
printf "EOF" > "$empty_blocks"

./work1_tc --help > "$help_out"
grep -q "work1_tc encrypt" "$help_out"
grep -q "work1_tc genkey" "$help_out"
expect_fail "legacy help exits nonzero by historical CLI convention" \
    "$legacy_help_out" ./work1 --help
grep -q "genkey" "$legacy_help_out"

expect_fail "work1_tc unknown command" "$arg_out" ./work1_tc unknown
expect_fail "work1 unknown command" "$arg_out" ./work1 unknown
expect_fail "work1_tc wrong option name" "$arg_out" \
    ./work1_tc encrypt --bad "$plain" --pubkey "$small_pub" --outfile "$tc_cipher"
expect_fail "work1 wrong option name" "$arg_out" \
    ./work1 encrypt --bad "$plain" --pubkey "$small_pub" --outfile "$legacy_cipher"
expect_fail "work1_tc too few args" "$arg_out" ./work1_tc encrypt
expect_fail "work1 too few args" "$arg_out" ./work1 encrypt
expect_fail "work1_tc too many args" "$arg_out" \
    ./work1_tc encrypt --infile "$plain" --pubkey "$small_pub" --outfile "$tc_cipher" extra
expect_fail "work1 too many args" "$arg_out" \
    ./work1 encrypt --infile "$plain" --pubkey "$small_pub" --outfile "$legacy_cipher" extra
expect_fail "work1_tc invalid key size" "$arg_out" \
    ./work1_tc genkey --size 123 --pubkey "$tmp_dir/bad_size_pub.txt" --secret "$tmp_dir/bad_size_sec.txt"
expect_fail "work1 invalid key size" "$arg_out" \
    ./work1 genkey --size 123 --pubkey "$tmp_dir/bad_size_pub.txt" --secret "$tmp_dir/bad_size_sec.txt"

expect_fail "work1_tc missing infile" "$missing_out" \
    ./work1_tc encrypt --infile "$tmp_dir/no_such_infile.txt" --pubkey "$small_pub" --outfile "$tc_cipher"
expect_fail "work1 missing infile" "$missing_out" \
    ./work1 encrypt --infile "$tmp_dir/no_such_infile.txt" --pubkey "$small_pub" --outfile "$legacy_cipher"
expect_fail "work1_tc missing pubkey" "$missing_out" \
    ./work1_tc encrypt --infile "$plain" --pubkey "$tmp_dir/no_such_pub.txt" --outfile "$tc_cipher"
expect_fail "work1 missing pubkey" "$missing_out" \
    ./work1 encrypt --infile "$plain" --pubkey "$tmp_dir/no_such_pub.txt" --outfile "$legacy_cipher"
expect_fail "work1_tc missing secret" "$missing_out" \
    ./work1_tc decrypt --infile "$empty_blocks" --secret "$tmp_dir/no_such_sec.txt" --outfile "$tc_decrypted"
expect_fail "work1 missing secret" "$missing_out" \
    ./work1 decrypt --infile "$empty_blocks" --secret "$tmp_dir/no_such_sec.txt" --outfile "$legacy_decrypted"
expect_fail "work1_tc missing sigfile" "$missing_out" \
    ./work1_tc check --infile "$plain" --pubkey "$small_pub" --sigfile "$tmp_dir/no_such_sig.txt"
expect_fail "work1 missing sigfile" "$missing_out" \
    ./work1 check --infile "$plain" --pubkey "$small_pub" --sigfile "$tmp_dir/no_such_sig.txt"

cat > "$tmp_dir/key_missing_n.txt" <<'KEY'
_x_bkm#
_e_bb#
&
KEY
cat > "$tmp_dir/key_missing_e.txt" <<'KEY'
_n_bkm#
_x_bb#
&
KEY
cat > "$tmp_dir/key_missing_d.txt" <<'KEY'
_n_bkm#
_x_bmk#
&
KEY
cat > "$tmp_dir/key_missing_hash.txt" <<'KEY'
_n_bkm
_e_bb#
&
KEY
cat > "$tmp_dir/key_missing_amp.txt" <<'KEY'
_n_bkm#
_e_bb#
KEY
cat > "$tmp_dir/key_invalid_payload.txt" <<'KEY'
_n_bqm#
_e_bb#
&
KEY

# Legacy work1 read_key() parses key files by fixed offsets and does not
# validate _n_/_e_/_d_ prefixes or payload characters. Keep strict corrupt-key
# rejection covered by work1_tc here; legacy corrupt block/signature files are
# still tested below through read_num_from_file(), which does validate prefixes.
expect_fail "work1_tc corrupt pubkey missing _n_" "$corrupt_out" \
    ./work1_tc encrypt --infile "$plain" --pubkey "$tmp_dir/key_missing_n.txt" --outfile "$tc_cipher"
expect_fail "work1_tc corrupt pubkey missing _e_" "$corrupt_out" \
    ./work1_tc encrypt --infile "$plain" --pubkey "$tmp_dir/key_missing_e.txt" --outfile "$tc_cipher"
expect_fail "work1_tc corrupt secret missing _d_" "$corrupt_out" \
    ./work1_tc decrypt --infile "$empty_blocks" --secret "$tmp_dir/key_missing_d.txt" --outfile "$tc_decrypted"
expect_fail "work1_tc corrupt key missing #" "$corrupt_out" \
    ./work1_tc encrypt --infile "$plain" --pubkey "$tmp_dir/key_missing_hash.txt" --outfile "$tc_cipher"
expect_fail "work1_tc corrupt key missing &" "$corrupt_out" \
    ./work1_tc encrypt --infile "$plain" --pubkey "$tmp_dir/key_missing_amp.txt" --outfile "$tc_cipher"
expect_fail "work1_tc corrupt key invalid payload char" "$corrupt_out" \
    ./work1_tc encrypt --infile "$plain" --pubkey "$tmp_dir/key_invalid_payload.txt" --outfile "$tc_cipher"

cat > "$tmp_dir/cipher_missing_c.txt" <<'CIPHER'
_x_a#
EOF
CIPHER
cat > "$tmp_dir/cipher_missing_hash.txt" <<'CIPHER'
_c_a
EOF
CIPHER
cat > "$tmp_dir/cipher_empty_block.txt" <<'CIPHER'
_c_#
EOF
CIPHER
cat > "$tmp_dir/cipher_invalid_payload.txt" <<'CIPHER'
_c_q#
EOF
CIPHER

for corrupt_cipher in \
    "$tmp_dir/cipher_missing_c.txt" \
    "$tmp_dir/cipher_missing_hash.txt" \
    "$tmp_dir/cipher_invalid_payload.txt"
do
    expect_fail "work1_tc corrupt cipher $(basename "$corrupt_cipher")" \
        "$corrupt_out" ./work1_tc decrypt --infile "$corrupt_cipher" --secret "$small_sec" --outfile "$tc_decrypted"
    expect_fail "work1 corrupt cipher $(basename "$corrupt_cipher")" \
        "$corrupt_out" ./work1 decrypt --infile "$corrupt_cipher" --secret "$small_sec" --outfile "$legacy_decrypted"
done
expect_fail "work1_tc corrupt cipher empty block" "$corrupt_out" \
    ./work1_tc decrypt --infile "$tmp_dir/cipher_empty_block.txt" --secret "$small_sec" --outfile "$tc_decrypted"

./work1_tc genkey --size 256 --pubkey "$generated_pub" --secret "$generated_sec"
./work1_tc decrypt --infile "$empty_blocks" --secret "$generated_sec" --outfile "$tc_empty_decrypted"
cmp "$empty_plain" "$tc_empty_decrypted"
./work1_tc check --infile "$empty_plain" --pubkey "$generated_pub" --sigfile "$empty_blocks" > "$tc_empty_check_out"
grep -q "File signature is correct!" "$tc_empty_check_out"
./work1 decrypt --infile "$empty_blocks" --secret "$generated_sec" --outfile "$legacy_empty_decrypted"
cmp "$empty_plain" "$legacy_empty_decrypted"
./work1 check --infile "$empty_plain" --pubkey "$generated_pub" --sigfile "$empty_blocks" > "$legacy_empty_check_out"
grep -q "File signature is correct!" "$legacy_empty_check_out"

./work1_tc encrypt --infile "$plain" --pubkey "$small_pub" --outfile "$tc_cipher"
./work1 decrypt --infile "$tc_cipher" --secret "$small_sec" --outfile "$legacy_decrypted"
cmp "$plain" "$legacy_decrypted"

./work1 encrypt --infile "$plain" --pubkey "$small_pub" --outfile "$legacy_cipher"
./work1_tc decrypt --infile "$legacy_cipher" --secret "$small_sec" --outfile "$tc_decrypted"
cmp "$plain" "$tc_decrypted"

./work1_tc sign --infile "$plain" --secret "$small_sec" --sigfile "$tc_sig"
./work1 check --infile "$plain" --pubkey "$small_pub" --sigfile "$tc_sig" > "$legacy_check_out"
grep -q "File signature is correct!" "$legacy_check_out"

./work1 sign --infile "$plain" --secret "$small_sec" --sigfile "$legacy_sig"
./work1_tc check --infile "$plain" --pubkey "$small_pub" --sigfile "$legacy_sig" > "$tc_check_out"
grep -q "File signature is correct!" "$tc_check_out"

./work1_tc check --infile "$tampered_plain" --pubkey "$small_pub" --sigfile "$legacy_sig" > "$tc_check_tampered_out"
grep -q "File signature is NOT correct!" "$tc_check_tampered_out"
expect_check_result "legacy check rejects modified plaintext" \
    "File signature is NOT correct!" "$legacy_check_tampered_out" \
    ./work1 check --infile "$tampered_plain" --pubkey "$small_pub" --sigfile "$tc_sig"
expect_check_result "work1_tc check rejects appended plaintext tail" \
    "File signature is NOT correct!" "$tc_check_appended_out" \
    ./work1_tc check --infile "$appended_plain" --pubkey "$small_pub" --sigfile "$legacy_sig"
expect_check_result "legacy check rejects appended plaintext tail" \
    "File signature is NOT correct!" "$legacy_check_appended_out" \
    ./work1 check --infile "$appended_plain" --pubkey "$small_pub" --sigfile "$tc_sig"
expect_check_result "work1_tc check rejects wrong public key" \
    "File signature is NOT correct!" "$tc_check_wrong_pub_out" \
    ./work1_tc check --infile "$plain" --pubkey "$wrong_pub" --sigfile "$legacy_sig"
expect_check_result "legacy check rejects wrong public key" \
    "File signature is NOT correct!" "$legacy_check_wrong_pub_out" \
    ./work1 check --infile "$plain" --pubkey "$wrong_pub" --sigfile "$tc_sig"

cat > "$bad_sig" <<'SIG'
_c_a#
EOF
SIG
./work1_tc check --infile "$plain" --pubkey "$small_pub" --sigfile "$bad_sig" > "$tc_check_bad_sig_out"
grep -q "File signature is NOT correct!" "$tc_check_bad_sig_out"
expect_check_result "legacy check rejects modified signature" \
    "File signature is NOT correct!" "$legacy_check_bad_sig_out" \
    ./work1 check --infile "$plain" --pubkey "$small_pub" --sigfile "$bad_sig"
expect_fail "work1_tc corrupt signature missing _c_" "$corrupt_out" \
    ./work1_tc check --infile "$plain" --pubkey "$small_pub" --sigfile "$tmp_dir/cipher_missing_c.txt"
expect_fail "work1 corrupt signature missing _c_" "$corrupt_out" \
    ./work1 check --infile "$plain" --pubkey "$small_pub" --sigfile "$tmp_dir/cipher_missing_c.txt"
expect_fail "work1_tc corrupt signature missing #" "$corrupt_out" \
    ./work1_tc check --infile "$plain" --pubkey "$small_pub" --sigfile "$tmp_dir/cipher_missing_hash.txt"
expect_fail "work1 corrupt signature missing #" "$corrupt_out" \
    ./work1 check --infile "$plain" --pubkey "$small_pub" --sigfile "$tmp_dir/cipher_missing_hash.txt"
expect_fail "work1_tc corrupt signature invalid payload char" "$corrupt_out" \
    ./work1_tc check --infile "$plain" --pubkey "$small_pub" --sigfile "$tmp_dir/cipher_invalid_payload.txt"
expect_fail "work1 corrupt signature invalid payload char" "$corrupt_out" \
    ./work1 check --infile "$plain" --pubkey "$small_pub" --sigfile "$tmp_dir/cipher_invalid_payload.txt"
expect_check_result "work1_tc empty signature block is semantic rejection" \
    "File signature is NOT correct!" "$tc_check_modified_sig_out" \
    ./work1_tc check --infile "$plain" --pubkey "$small_pub" --sigfile "$tmp_dir/cipher_empty_block.txt"
expect_check_result "legacy empty signature block is semantic rejection" \
    "File signature is NOT correct!" "$legacy_check_modified_sig_out" \
    ./work1 check --infile "$plain" --pubkey "$small_pub" --sigfile "$tmp_dir/cipher_empty_block.txt"

cp "$tc_cipher" "$tmp_dir/tc_cipher_trailing.txt"
printf '\nTRAILING-GARBAGE\n' >> "$tmp_dir/tc_cipher_trailing.txt"
./work1_tc decrypt --infile "$tmp_dir/tc_cipher_trailing.txt" --secret "$small_sec" --outfile "$tc_decrypt_trailing_out"
cmp "$plain" "$tc_decrypt_trailing_out"
expect_fail "legacy rejects newline/trailing data after EOF marker" \
    "$corrupt_out" ./work1 decrypt --infile "$tmp_dir/tc_cipher_trailing.txt" --secret "$small_sec" --outfile "$legacy_decrypt_trailing_out"

echo "work1_tc smoke test PASSED"
