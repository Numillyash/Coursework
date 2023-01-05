#!/bin/bash
#./Coursework.exe genkey --size 256 --pubkey pb_256.txt --secret sk_256.txt
#./Coursework.exe encrypt --infile test_inp_1.txt --pubkey pb_256.txt --outfile test_out.txt
./Coursework.exe decrypt --infile test_out.txt --secret sk_256.txt --outfile test_out2.txt
#./Coursework.exe encrypt --infile test_inp_1.txt --pubkey pb_test.txt --outfile test_out.txt
#./Coursework.exe decrypt --infile test_out.txt --secret sk_test.txt --outfile test_out2.txt