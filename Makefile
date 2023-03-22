CC = gcc
COMPILER_FLAGS = -Wall -Wextra -Wpedantic
SOURCES = ./src/*.c
INCLUDES = ./include/
LINKER_FLAGS = -lncurses 
OBJ_NAME = ./build/toscop


all:
	mkdir -p build/
	$(CC) $(SOURCES) $(COMPILER_FLAGS) $(LINKER_FLAGS) -I $(INCLUDES) -o $(OBJ_NAME)

clean:
	rm -f $(OBJ_NAME)

run:
	@$(OBJ_NAME)

