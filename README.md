# Code
******************************************************************************************************************
#### README FOR CHAT APPLICATION
******************************************************************************************************************

------------------------------------------ Descripton of s-talk ----------------------------------------------

This program contains 6 files - zipped in the folder `s-talk`:
- `s-talk.c`
- `transmit.c`
- `transmit.h`
- `list.h`
- `list.o` (from the instructor)
- `makefile`

---------------------------------------- Instruction for makefile --------------------------------------------

- `make all`: Compile all the files for general use.
- `make demo`: Run s-talk with the following parameters:
  - Local PORT: 6060
  - Remote machine name: localhost
  - Remote PORT: 6061
- `make valgrind`: Run in valgrind mode to check for memory leaks.
- `make clean`: Clean executable files of the s-talk program.

---------------------------------------- Major Problems of s-talk -------------------------------------------

**Memory leaks problem:**
- S-talk still retains one allocation memory that cannot be freed.

**Closing s-talk problem:**
- S-talk fails to terminate due to invalid access to `free()`; it tries to free an empty memory.

##############################################################################################################
##############################################################################################################
