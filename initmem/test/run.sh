make all
mv ./assn_4 ./test_samples/

pushd ./test_samples

rm index.bin
./assn_4 index.bin 3 < input3.txt > my3.txt
diff output3.txt my3.txt
echo "Order 3 done"

rm index.bin
./assn_4 index.bin 4 < input4.txt > my4.txt
diff output4.txt my4.txt
echo "Order 4 done"

rm index.bin
./assn_4 index.bin 10 < input10a.txt > my10a.txt
diff output10a.txt my10a.txt
echo "Order 10 done"

#rm index.bin
./assn_4 index.bin 10 < input10b.txt > my10b.txt
diff output10b.txt my10b.txt
echo "Order 10 done"

rm index.bin
./assn_4 index.bin 20 < input20.txt > my20.txt
diff output20.txt my20.txt
echo "Order 20 done"

rm index.bin
./assn_4 index.bin 4 < input1a.txt > my1a.txt
diff output1a.txt my1a.txt
echo "Order 4 done"

#rm index.bin
./assn_4 index.bin 4 < input2a.txt > my2a.txt
diff output2a.txt my2a.txt
echo "Order 4 persistence done"

rm ./assn_4
popd
