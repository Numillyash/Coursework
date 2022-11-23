while read -r a
do
while read -r b
do
    keysize=$((a))
    num=$((b))
    file_out="./test_results/test_outputs/test_dec_${num}_${keysize}_($2).txt"
    file_inp="./test_results/tmp/test_tmp_enc_${num}_${keysize}_($2).txt"
    file_sec="./test_results/keys/sc$keysize.txt"
    START_TIME=$(date +%s)
    ./work1 decrypt --infile $file_inp --secret $file_sec --outfile $file_out
    exit_code=$?
    END_TIME=$(date +%s)
    DIFF=$(( $END_TIME - $START_TIME ))
    echo "Runtime: exit_code: $exit_code, decrypt $num file, bit: $keysize, time: $DIFF" >> ./test_results/logs/"$1".txt
done < './tests/file_nums'
done < './tests/keysizes'