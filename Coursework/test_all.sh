#!/usr/bin/env bash
set -euo pipefail

mkdir -p \
    test_results/logs \
    test_results/keys \
    test_results/test_inputs \
    test_results/test_outputs \
    test_results/tmp

make MODE="${MODE:-debug}" all

filename=$(date +"%F_%H-%M-%S")
echo $filename
failed=0

check_log_success() {
    local log_file="./test_results/logs/$1.txt"

    if [ ! -f "$log_file" ]
    then
        echo "Missing log file: $log_file" >&2
        return 1
    fi
    if grep -Eq 'exit_code: [1-9][0-9]*' "$log_file"
    then
        echo "Command failure recorded in $log_file" >&2
        return 1
    fi
}

for i in {1..1}
do
    start=$(date +%s.%N)

    bash ./tests/test_genkey.sh "keygen-$filename"
    check_log_success "keygen-$filename"

    bash ./tests/test_encrypt.sh "enc-$filename" $i
    check_log_success "enc-$filename"

    bash ./tests/test_decrypt.sh "dec-$filename" $i
    check_log_success "dec-$filename"

    while read -r keysize
    do
        while read -r num
        do
            file_inp="./test_results/test_inputs/test_inp_$num.txt"
            file_out="./test_results/test_outputs/test_dec_${num}_${keysize}_($i).txt"
            if ! cmp -s "$file_inp" "$file_out"
            then
                echo "Mismatch: $file_out differs from $file_inp" >&2
                failed=1
            fi
        done < './tests/file_nums'
    done < './tests/keysizes'

    bash ./tests/test_sign.sh "sign-$filename"
    check_log_success "sign-$filename"

    bash ./tests/test_check.sh "check-$filename"
    check_log_success "check-$filename"

    end=$(date +%s.%N)
    DIFF=$( echo "$end - $start" | bc -l )
    echo "Runtime: ALL, time: $DIFF" >> ./test_results/logs/"ALL=$filename".txt
done

exit "$failed"
