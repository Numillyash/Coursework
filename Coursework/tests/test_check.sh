#!/bin/bash
deg=1
while read -r a
do
while read -r b
do
    keysize=$((a))
    num=$((b))
    file_sig="./test_results/tmp/test_tmp_sig_${num}_$keysize.txt"
    file_inp="./test_results/test_inputs/test_inp_$num.txt"
    file_key="./test_results/keys/pb$keysize.txt"
    start=`date +%s.%N`
    ./work1 check --infile $file_inp --pubkey $file_key --sigfile $file_sig
    exit_code=$?
    end=`date +%s.%N`
    runtime=$( echo "$end - $start" | bc -l )
    echo Runtime: exit_code: $exit_code, check $num file, bit: $keysize, time: $runtime >> ./test_results/logs/"$1".txt
done < './tests/file_nums'
done < './tests/keysizes'