CC=gcc
SRC=src/compiler.c src/x86.c src/frontend.c src/bytecode.c src/utils.c
OBJ = $(SRC:.c=.o)
CFLAGS=-Wall -Wextra -Werror -ggdb3
LFLAGS=-ggdb3
SANS=-fsanitize=address,undefined

all: compiler

# hello: hello.o
# 	ld hello.o -o hello -dynamic-linker /lib64/ld-linux-x86-64.so.2 -lc -m elf_x86_64 /usr/lib/x86_64-linux-gnu/crt1.o /usr/lib/x86_64-linux-gnu/crti.o  /usr/lib/x86_64-linux-gnu/crtn.o -m elf_x86_64
#
# hello.o: hello.asm
# 	nasm -o hello.o -felf64 hello.asm
#
# hello.asm: compiler
# 	./compiler sample.c > hello.asm


compiler: $(OBJ)
	$(CC) $(LFLAGS) -o $@ $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) compiler
