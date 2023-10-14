# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -pedantic -std=c99

TARGET = main
HEADER = clientServer.h
DEP = clientServer.o
DEPSRC = clientServer.c
SRC = main.c

$(TARGET): $(SRC) $(HEADER) $(DEP)
	$(CC) -o $(TARGET) $(SRC) $(DEP)
	make run

$(DEP): $(DEPSRC) $(HEADER)
	$(CC) -c $(DEPSRC)

clean:
	rm -f $(DEP)

run: $(TARGET) clean
	clear
	./$(TARGET)