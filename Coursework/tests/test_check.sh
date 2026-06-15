set -u

log_name="$1"

run_check_case() {
    local label="$1"
    local expected="$2"
    local file_inp="$3"
    local file_key="$4"
    local file_sig="$5"
    local log_file="./test_results/logs/$log_name.txt"
    local out_file="./test_results/tmp/check_${label}.out"
    local start
    local end
    local diff
    local exit_code

    start=$(date +%s.%N)
    ./work1 check --infile "$file_inp" --pubkey "$file_key" --sigfile "$file_sig" > "$out_file" 2>&1
    exit_code=$?
    end=$(date +%s.%N)
    diff=$(echo "$end - $start" | bc -l)

    echo "Runtime: exit_code: $exit_code, check $label, time: $diff" >> "$log_file"

    if [ "$exit_code" -ne 0 ]
    then
        echo "FAIL: check $label exited with $exit_code" >&2
        cat "$out_file" >&2
        return 1
    fi

    if ! grep -Fq "$expected" "$out_file"
    then
        echo "FAIL: check $label did not print expected result: $expected" >&2
        cat "$out_file" >&2
        return 1
    fi
}

make_signature_with_extra_block() {
    local src="$1"
    local dst="$2"

    awk '
        /^EOF$/ {
            print "_c_a#"
            print "EOF"
            next
        }
        { print }
    ' "$src" > "$dst"
}

while read -r a
do
while read -r b
do
    keysize=$((a))
    num=$((b))
    file_sig="./test_results/tmp/test_tmp_sig_${num}_$keysize.txt"
    file_inp="./test_results/test_inputs/test_inp_$num.txt"
    file_key="./test_results/keys/pb$keysize.txt"
    wrong_pub="./test_results/tmp/wrong_pb$keysize.txt"
    wrong_sec="./test_results/tmp/wrong_sc$keysize.txt"
    sig_modified="./test_results/tmp/test_tmp_sig_${num}_${keysize}_modified.txt"
    inp_appended="./test_results/tmp/test_inp_${num}_${keysize}_appended.txt"

    run_check_case "${num}_${keysize}_valid" \
        "File signature is correct!" \
        "$file_inp" "$file_key" "$file_sig" || exit 1

    make_signature_with_extra_block "$file_sig" "$sig_modified"
    run_check_case "${num}_${keysize}_modified_sig" \
        "File signature is NOT correct!" \
        "$file_inp" "$file_key" "$sig_modified" || exit 1

    cp "$file_inp" "$inp_appended"
    printf 'tail-after-signing' >> "$inp_appended"
    run_check_case "${num}_${keysize}_appended_input" \
        "File signature is NOT correct!" \
        "$inp_appended" "$file_key" "$file_sig" || exit 1

    if [ ! -f "$wrong_pub" ] || [ ! -f "$wrong_sec" ]
    then
        ./work1 genkey --size "$keysize" --pubkey "$wrong_pub" --secret "$wrong_sec" > ./test_results/tmp/wrong_keygen_"$keysize".out 2>&1
        exit_code=$?
        echo "Runtime: exit_code: $exit_code, mode: wrong-keygen, bit: $keysize" >> ./test_results/logs/"$log_name".txt
        if [ "$exit_code" -ne 0 ]
        then
            echo "FAIL: wrong-key keygen exited with $exit_code" >&2
            cat ./test_results/tmp/wrong_keygen_"$keysize".out >&2
            exit 1
        fi
    fi

    run_check_case "${num}_${keysize}_wrong_pubkey" \
        "File signature is NOT correct!" \
        "$file_inp" "$wrong_pub" "$file_sig" || exit 1
done < './tests/file_nums'
done < './tests/keysizes'
