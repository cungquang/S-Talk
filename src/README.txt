**************************************************************************************************************
******              	  										    ******
******                  			README FOR S-TALK 				 	    ******
******													    ******		
******--------------------------------------------------------------------------------------------------******
******--------------------------------------------------------------------------------------------------******
******              	  										    ******
******						COURSE: CMPT-300					    ******
******						STUDENT NAME: HONG QUANG CUNG				    ******
******						STUDENT ID: 301417603					    ******
******              	  										    ******
**************************************************************************************************************

------------------------------------------ Descripton of s-talk ----------------------------------------------


This program contain 6 files - zip in folder s-talk
- s-talk.c
- transmit.c
- transmit.h
- list.h
- list.o (from instructor)
- makefile


---------------------------------------- Instruction for makefile --------------------------------------------


make all - compile all the file for general use
make demo - this will run s-talk with: Local PORT: 6060 - Remote machine name: localhost - Remote PORT: 6061
make valgrind - run in valgrind mode to check memory leak
make clean - clean executable file of s-talk program


---------------------------------------- Major Problems of s-talk --------------------------------------------


Memory leaks problem:
- s-talk still remain 1 allocation memory can not be free
Closing s-talk problem:
- s-talk fail to terminate due to invalid access to free() - it try to free an empty memory


##############################################################################################################
##############################################################################################################

