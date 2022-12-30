make
#cd tests
# mkdir test_results
# mkdir test_results/keys
# mkdir test_results/logs
# mkdir test_results/test_inputs
# mkdir test_results/test_outputs
# mkdir test_results/tmp


filename=$(date +"%F %X")
echo $filename
for i in {1..1}
do
    start=`date +%s.%N`
    #START_TIME=$(date +%s)
    #
    bash ./tests/test_genkey.sh "keygen-$filename" 
    sleep 2
    # bash ./tests/test_sign.sh "sign-$filename"
    # bash ./tests/test_check.sh "check-$filename"
    bash ./tests/test_encrypt.sh "enc-$filename" $i
    bash ./tests/test_decrypt.sh "dec-$filename" $i #> a2.txt
    #
    end=`date +%s.%N`
    #END_TIME=$(date +%s)
        
    #DIFF=$(( $END_TIME - $START_TIME ))
    DIFF=$( echo "$end - $start" | bc -l )
    echo "Runtime: ALL, time: $DIFF" >> ./test_results/logs/"ALL=$filename".txt
    tail a2.txt
done