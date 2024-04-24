This project implements a basic shell that allows the user to execute Linux commands. 
All Linux commands were implemented using system calls such as write, open, close etc. 
The shell has two modes, a file mode that reads all commands from an input file and an interactive 
mode that lets the user dynamically execute commands one after another until they exit the shell. 
This program has 3 different components, a main function that loops through input and directs the 
flow of the program, a parser function called by main to tokenize the input, and lastly, the implementations 
of each Linux command using only system calls called by main after input has been parsed. This project also 
implemented Error checking for invalid files, incorrect number of inputs as well as invalid commands. 