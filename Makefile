CC = gcc
COMPILER_FLAGS = -g -Wall -Wextra -Wpedantic
SOURCES = ./src/*.c
INCLUDES = ./include/
LINKER_FLAGS = -lncurses -lpthread 
NAME = toscop

$(NAME):
	mkdir -p build/
	$(CC) $(SOURCES) $(COMPILER_FLAGS) $(LINKER_FLAGS) -I $(INCLUDES) -o ./build/$(NAME)

run: $(NAME) 
	@./build/$(NAME)

clean:
	rm -f ./build/$(NAME)
	rmdir ./build

