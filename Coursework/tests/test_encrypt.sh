while read -r a
do
while read -r b
do
    keysize=$((a))
    num=$((b))
    file_inp="./test_results/test_inputs/test_inp_$num.txt"
    file_out="./test_results/tmp/test_tmp_enc_${num}_${keysize}_($2).txt"
    file_key="./test_results/keys/pb$keysize.txt"

    start=`date +%s.%N`
    #START_TIME=$(date +%s)
    
    ./work1 encrypt --infile $file_inp --pubkey $file_key --outfile $file_out
    exit_code=$?

    end=`date +%s.%N`
    #END_TIME=$(date +%s)
    
    #DIFF=$(( $END_TIME - $START_TIME ))
    DIFF=$( echo "$end - $start" | bc -l )

    echo "Runtime: exit_code: $exit_code, encrypt $num file, bit: $keysize, time: $DIFF" >> ./test_results/logs/"$1".txt
done < './tests/file_nums'
done < './tests/keysizes'