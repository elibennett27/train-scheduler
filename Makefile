all: mts

mts: mts.c
	gcc -pthread mts.c -o mts


clean:
	rm -f *.o
	rm -f mts
	rm -f output.txt
