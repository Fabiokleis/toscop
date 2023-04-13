CC = gcc
COMPILER_FLAGS = -ggdb -Wall -Wextra -Wpedantic -std=c11
SOURCES = ./src/*.c
INCLUDES = -I ./include/
LINKER_FLAGS = -lncurses -lpthread 
NAME = toscop

$(NAME):
	mkdir -p build/
	$(CC) $(SOURCES) $(COMPILER_FLAGS) $(LINKER_FLAGS) $(INCLUDES) -o ./build/$(NAME)

run: $(NAME) 
	@./build/$(NAME)

install: $(NAME)
	install -m 0755 ./build/$(NAME) /usr/local/sbin/$(NAME)

clean:
	rm -f ./build/$(NAME)
	rmdir ./build

