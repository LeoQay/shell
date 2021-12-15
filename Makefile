NAME=solution
FLAGS= -O2 -ftrapv -fsanitize=address,undefined -Wall -Werror -Wformat-security -Wignored-qualifiers -Winit-self -Wswitch-default -Wfloat-equal -Wshadow -Wpointer-arith -Wtype-limits -Wempty-body -Wlogical-op -Wstrict-prototypes -Wold-style-declaration -Wold-style-definition -Wmissing-parameter-type -Wmissing-field-initializers -Wnested-externs -Wno-pointer-sign -std=gnu11 -lm
IN_END=

all: driver.o launcher.o list.o parser.o process.o str.o token.o
	gcc driver.o launcher.o list.o parser.o process.o str.o token.o -o $(NAME) $(IN_END)

run:
	./$(NAME)

driver.o: driver.c launcher.h list.h str.h process.h error.h parser.h
	gcc driver.c -c
launcher.o: launcher.c error.h launcher.h list.h str.h process.h
	gcc launcher.c -c $(IN_END)

list.o: list.c  list.h str.h token.h
	gcc list.c -c $(IN_END)

parser.o: parser.c  launcher.h list.h str.h process.h error.h parser.h token.h
	gcc parser.c -c $(IN_END)

process.o: process.c process.h list.h str.h
	gcc process.c -c $(IN_END)

str.o: str.c str.h
	gcc str.c -c $(IN_END)

token.o: token.c token.h str.h list.h
	gcc token.c -c $(IN_END)

clean:
	rm -f *.o $(NAME)
