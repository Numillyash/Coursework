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
help_out="$tmp_dir/help.out"
genkey_out="$tmp_dir/genkey.out"

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

printf "A" > "$plain"
printf "B" > "$tampered_plain"
: > "$empty_plain"
printf "EOF" > "$empty_blocks"

./work1_tc --help > "$help_out"
grep -q "work1_tc encrypt" "$help_out"

if ./work1_tc genkey --size 256 --pubkey "$generated_pub" --secret "$generated_sec" > "$genkey_out" 2>&1; then
    echo "work1_tc genkey unexpectedly succeeded" >&2
    exit 1
fi
grep -q "genkey is not supported by work1_tc yet" "$genkey_out"

./work1 genkey --size 256 --pubkey "$generated_pub" --secret "$generated_sec"
./work1_tc decrypt --infile "$empty_blocks" --secret "$generated_sec" --outfile "$tc_empty_decrypted"
cmp "$empty_plain" "$tc_empty_decrypted"
./work1_tc check --infile "$empty_plain" --pubkey "$generated_pub" --sigfile "$empty_blocks" > "$tc_empty_check_out"
grep -q "File signature is correct!" "$tc_empty_check_out"

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

cat > "$bad_sig" <<'SIG'
_c_a#
EOF
SIG
./work1_tc check --infile "$plain" --pubkey "$small_pub" --sigfile "$bad_sig" > "$tc_check_bad_sig_out"
grep -q "File signature is NOT correct!" "$tc_check_bad_sig_out"

echo "work1_tc smoke test PASSED"
