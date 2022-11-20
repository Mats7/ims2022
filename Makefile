CC=g++
FLAGS=-std=gnu++11 -Wall -pedantic

all:
	$(CC) $(FLAGS) ims.cpp -o ims -l simlib -lm
run: clean all
	./ims
clean:
	rm -f ./ims
zip:
	zip 11_xcimme00_xpaulo04.zip ims.cpp Makefile dokumentacia.pdf

