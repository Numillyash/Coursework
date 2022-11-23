make
#cd tests
mkdir test_results
mkdir test_results/keys
mkdir test_results/logs
mkdir test_results/test_inputs
mkdir test_results/test_outputs
mkdir test_results/tmp

filename=$(date +"%F %X")
echo $filename
for i in {1..10}:
do
START_TIME=$(date +%s)
#
bash ./tests/test_genkey.sh "keygen-$filename" 
# bash ./tests/test_sign.sh "sign-$filename"
# bash ./tests/test_check.sh "check-$filename"
bash ./tests/test_encrypt.sh "enc-$filename"
bash ./tests/test_decrypt.sh "dec-$filename"
#
END_TIME=$(date +%s)
DIFF=$(( $END_TIME - $START_TIME ))
echo "Runtime: ALL, time: $DIFF" >> ./test_results/logs/"ALL=$filename".txt
done