all : ghost-shell
	

ghost-shell: main.o string_parser.o 
	gcc -g -o ghost-shell main.o string_parser.o
	gcc iobound.c -o iobound
	gcc cpubound.c -o cpubound
	
main.o: main.c
	gcc -g -c main.c
	
string_parser.o: string_parser.c string_parser.h
	gcc -g -c string_parser.c
	
clean:
	rm -f core *.o ghost-shell iobound cpubound
