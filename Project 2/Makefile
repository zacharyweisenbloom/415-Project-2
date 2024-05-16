all : part1_m
	

part1_m: part4.o string_parser.o 
	gcc -g -o part1_m part4.o string_parser.o
	gcc iobound.c -o iobound
	gcc cpubound.c -o cpubound
	
part4.o: part4.c
	gcc -g -c part4.c
	
string_parser.o: string_parser.c string_parser.h
	gcc -g -c string_parser.c
	
clean:
	rm -f core *.o part1_m iobound cpubound
