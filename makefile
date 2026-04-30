OBJ:=$(patsubst %.c,%.o,$(wildcard *.c));
stego:$(OBJ)
	gcc -o $@ $^
clean:
	rm -f stego *.o