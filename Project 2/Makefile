all : part1_m part2_m part3_m part4_m iobound cpubound

iobound:
	gcc iobound.c -o iobound

cpubound:
	gcc cpubound.c -o cpubound

part1_m: part1.o string_parser.o
	gcc -g -o part1_m part1.o string_parser.o
	
part1.o: part1.c
	gcc -g -c part1.c

part2_m: part2.o string_parser.o
	gcc -g -o part2_m part2.o string_parser.o
	
part2.o: part2.c
	gcc -g -c part2.c

part3_m: part3.o string_parser.o
	gcc -g -o part3_m part3.o string_parser.o
	
part3.o: part3.c
	gcc -g -c part3.c
	
part4_m: part4.o string_parser.o
	gcc -g -o part4_m part4.o string_parser.o
	
part4.o: part4.c
	gcc -g -c part4.c
	
string_parser.o: string_parser.c string_parser.h
	gcc -g -c string_parser.c
	
clean:
	rm -f core *.o part1_m part2_m part3_m part4_m iobound cpubound