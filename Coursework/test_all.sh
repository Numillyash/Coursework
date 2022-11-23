#make
#cd tests
# mkdir test_results
# mkdir test_results/keys
# mkdir test_results/logs
# mkdir test_results/test_inputs
# mkdir test_results/test_outputs
# mkdir test_results/tmp

filename=$(date +"%F %X")
echo $filename

bash ./tests/test_genkey.sh "keygen-$filename" 
bash ./tests/test_sign.sh "sign-$filename"
bash ./tests/test_check.sh "check-$filename"