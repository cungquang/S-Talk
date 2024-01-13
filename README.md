# S-TALK
******************************************************************************************************************

## Project Information

- **Genre:** Chat Application
- **Language:** C

## Project Description

The S-TALK project was developed for educational purposes, aiming to explore socket programming in C. The primary communication method utilized is UDP (User Datagram Protocol). The focus of the project is to create a simple chat application allowing two users to communicate over the internet.

- **Purpose:** Educational purpose
- **Target Audience:** Not Applicable
- **Hosting:** Local machine
- **Tech Stack:**
  - C

## Instruction for Makefile

This program consists of 6 files zipped in the folder `s-talk`:

- `s-talk.c`
- `transmit.c`
- `transmit.h`
- `list.h`
- `list.o` (provided by the instructor)
- `makefile`

**Commands for Makefile:**
- `make all`: Compile all the files for general use.
- `make demo`: Run S-TALK with the following parameters:
  - Local PORT: 6060
  - Remote machine name: localhost
  - Remote PORT: 6061
- `make valgrind`: Run in valgrind mode to check for memory leaks.
- `make clean`: Clean executable files of the S-TALK program.

## Potential Issues of S-TALK

### Closing S-TALK Problem

S-TALK may fail to terminate due to invalid access to `free()`, attempting to free unallocated memory.
