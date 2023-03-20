CC = gcc
COMPILER_FLAGS = -Wall
SOURCES = ./src/*.c
INCLUDES = ./include/
LINKER_FLAGS = 
OBJ_NAME = ./build/toscop


all:
	mkdir -p build/
	$(CC) $(SOURCES) $(COMPILER_FLAGS) $(LINKER_FLAGS) -I $(INCLUDES) -o $(OBJ_NAME)

clean:
	rm -f $(OBJ_NAME)

run:
	@$(OBJ_NAME)

