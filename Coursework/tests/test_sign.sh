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
    START_TIME=$(date +%s)
    ./work1.exe sign --infile $file_inp --secret $file_sec --sigfile $file_out
    exit_code=$?
    END_TIME=$(date +%s)
    DIFF=$(( $END_TIME - $START_TIME ))
    echo "Runtime: exit_code: $exit_code, sign $num file, bit: $keysize, time: $DIFF" >> ./test_results/logs/"$1".txt
done < './tests/file_nums'
done < './tests/keysizes'