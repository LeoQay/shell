NAME=solution

all: driver.o launcher.o list.o parser.o process.o str.o token.o
	gcc driver.o launcher.o list.o parser.o process.o str.o token.o -o $(NAME)

run:
	./$(NAME)

driver.o: driver.c launcher.h list.h str.h process.h error.h parser.h
	gcc driver.c -c

launcher.o: launcher.c error.h launcher.h list.h str.h process.h
	gcc launcher.c -c

list.o: list.c  list.h str.h token.h
	gcc list.c -c

parser.o: parser.c  launcher.h list.h str.h process.h error.h parser.h token.h
	gcc parser.c -c

process.o: process.c process.h list.h str.h
	gcc process.c -c

str.o: str.c str.h
	gcc str.c -c

token.o: token.c token.h str.h list.h
	gcc token.c -c

clean:
	rm -f *.o $(NAME)
