all : part1_m
	

part1_m: part2.o string_parser.o 
	gcc -g -o part1_m part2.o string_parser.o
	gcc iobound.c -o iobound
	gcc cpubound.c -o cpubound
	
part2.o: part2.c
	gcc -g -c part2.c
	
string_parser.o: string_parser.c string_parser.h
	gcc -g -c string_parser.c
	
clean:
	rm -f core *.o part1_m iobound cpubound
