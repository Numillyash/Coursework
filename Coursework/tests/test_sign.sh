#!/bin/bash
deg=1
while read -r a
do
while read -r b
do
    keysize=$((a))
    num=$((b))
    file_inp="./test_results/test_inputs/test_inp_$num.txt"
    file_out="./test_results/tmp/test_tmp_sig_${num}_$keysize.txt"
    file_sec="./test_results/keys/sc$keysize.txt"
    start=`date +%s.%N`
    ./work1 sign --infile $file_inp --secret $file_sec --sigfile $file_out
    exit_code=$?
    end=`date +%s.%N`
    runtime=$( echo "$end - $start" | bc -l )
    echo Runtime: exit_code: $exit_code, sign $num file, bit: $keysize, time: $runtime >> ./test_results/logs/"$1".txt
done < './tests/file_nums'
done < './tests/keysizes'