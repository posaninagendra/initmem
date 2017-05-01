all:assn_4.o data_structures.o
	gcc -g -o assn_4 assn_4.o data_structures.o -lm
assn_4.o:assn_4.c
	gcc -g -c assn_4.c -o assn_4.o
data_structures.o:data_structures.c
	gcc -g -c -w data_structures.c -o data_structures.o
clean:
	rm *.o
	rm assn_4
	rm index.bin
