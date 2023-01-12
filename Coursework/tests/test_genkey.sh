while read -r a
do
keysize=$((a))
file_inp="./test_results/keys/pb$keysize.txt"
file_out="./test_results/keys/sc$keysize.txt"

    START_TIME=$(date +%s)
./work1 genkey --size $keysize --pubkey $file_inp --secret $file_out
exit_code=$?
#runtime=$((end-start))
    END_TIME=$(date +%s)
    DIFF=$(( $END_TIME - $START_TIME ))
echo "Runtime: exit_code: $exit_code, mode: keygen, bit: $keysize, time: $DIFF" >> ./test_results/logs/"$1".txt;
done < './tests/keysizes'