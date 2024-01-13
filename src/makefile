CFLAGS = -Wall -g -std=c99 -D _POSIX_C_SOURCE=200809L -Werror

all:
	gcc $(CFLAGS) s-talk.c transmit.c list.o -lpthread -o s-talk

demo:
	./s-talk 6060 localhost 6061

valgrind:
	valgrind --leak-check=full ./s-talk 6060 localhost 6061

clean:
	rm -f s-talk
