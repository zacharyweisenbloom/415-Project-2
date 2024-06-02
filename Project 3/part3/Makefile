all: bank

bank: main.o string_parser.o 
	gcc -g -o bank main.o string_parser.o

main.o: main.c 
	gcc -g -c main.c
string_parser.o: string_parser.c
	gcc -g -c string_parser.c
	
