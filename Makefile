CC = gcc
COMPILER_FLAGS = -ggdb -Wall -Wextra -Wpedantic -std=c11
SOURCES = ./src/*.c
INCLUDES = -I ./include/
LINKER_FLAGS = -lncurses -lpthread 
NAME = toscop
TEX = $(NAME).tex

.PHONY: all
all: $(NAME) tex

$(NAME): build
	$(CC) $(SOURCES) $(COMPILER_FLAGS) $(LINKER_FLAGS) $(INCLUDES) -o ./build/$(NAME)

run: $(NAME) 
	@./build/$(NAME)

install: $(NAME)
	install -m 0755 ./build/$(NAME) /usr/local/sbin/$(NAME)

tex:
	pdflatex -shell-escape -interaction=batchmode $(TEX) 
	rm $(NAME).log 

build:
	@mkdir -p build/

clean:
	rm -f ./build/$(NAME)
	rmdir ./build

