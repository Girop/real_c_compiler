CC=clang
FLAGS=-Wall -Wextra -ggdb3 
SANS=-fsanitize=address,undefined

all: hello compiler

hello: hello.o
	ld hello.o -o hello -dynamic-linker /lib64/ld-linux-x86-64.so.2 -lc -m elf_x86_64 /usr/lib/x86_64-linux-gnu/crt1.o /usr/lib/x86_64-linux-gnu/crti.o  /usr/lib/x86_64-linux-gnu/crtn.o -m elf_x86_64

hello.o: hello.asm
	nasm -o hello.o -felf64 hello.asm

hello.asm: compiler
	./compiler sample.c > hello.asm

compiler: compiler.c
	$(CC) $(FLAGS) -o $@ $<


clean:
	rm compiler.o hello.asm hello.o hello
