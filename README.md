# Toscop
uma versão tosca do comando top,
trabalho para disciplina de sistemas operacionais.

![toscop4](https://user-images.githubusercontent.com/66813406/231803102-dae021aa-228b-4f96-8255-e3e38ff0d919.gif)

## Build e Run

Arquivo Makefile deste projeto:
```Makefile
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
```

Para compilar:
```bash
make
```
Para compilar e rodar:
```bash
make run
```

## Opções 
Para rodar com informações de debug:
```bash
./build/toscop -v
```
Para rodar com um refresh time específico (default 3):
```bash
./build/toscop -d5
```

## Install
Para instalar o toscop no `/usr/local/sbin/toscop`, a permissão fica 0755
```console
sudo make install
```

## Referências
- https://wiki.inf.ufpr.br/maziero/lib/exe/fetch.php?media=socm:socm-livro.pdf 
- https://man7.org/linux/man-pages/man2/syscalls.2.html                # lista com todas as syscalls
- https://man7.org/linux/man-pages/man2/sysinfo.2.html                 # struct para pegar informacoes gerais do sistema (memoria, swap, load avg...)
- https://pubs.opengroup.org/onlinepubs/7908799/xsh/dirent.h.html      # tipo do diretorio, utilizado para ler o /proc/[pid] 
- https://pubs.opengroup.org/onlinepubs/7908799/xsh/readdir.html       # funcao que retorna um struct dirent
- https://man7.org/linux/man-pages/man0/sys_stat.h.0p.html             # para entender como pegar informacoes de arquivo com a funcao stat 
- https://linux.die.net/man/3/snprintf                                 # para concatenar strings com formatacao
- https://linux.die.net/man/3/getpwuid                                 # para obter informacoes de usuario com base no uid
- https://man7.org/linux/man-pages/man2/nice.2.html                    # para enteder oque é nice
- https://linux.die.net/man/3/ctime                                    # para obter o tempo do sistema com o struct tm
- https://pubs.opengroup.org/onlinepubs/7908799/xcurses/curses.h.html  # ncurses para criar a interface do programa toscop
- https://man7.org/linux/man-pages/man5/proc.5.html                    # man do proc para ver quais arquivos preciso ler
- https://man7.org/linux/man-pages/man3/pthread_create.3.html          # para criar threads
- https://man7.org/linux/man-pages/man3/pthread_attr_init.3.html       # para criar os atributo para dar join
- https://linux.die.net/man/3/pthread_mutex_init                       # para entender as maneiras de inicializar mutex
- https://linux.die.net/man/3/pthread_mutex_lock                       # funcao para dar lock em um mutex
- https://linux.die.net/man/3/pthread_mutex_destroy                    # funcao para desalocar um mutex 
- https://pubs.opengroup.org/onlinepubs/7908799/xsh/time.h.html        # para entender os variados tipos de tempo e as maneiras de mensura-lo 
- https://man7.org/linux/man-pages/man3/clock_gettime.3.html           # funcao para obter o tempo a partir de um tipo de CLOCK 
- https://man7.org/linux/man-pages/man1/top.1.html                     # para entender como é calculado os valores de uso do cpu
- https://en.wikipedia.org/wiki/Load_%28computing%29                   # para entender como calcular o load average, ler o /usr/include/linux/sysinfo.h
- https://man7.org/linux/man-pages/man3/getline.3.html                 # funcao para ler apenas uma linha, troquei o fgets
- https://man7.org/linux/man-pages/man3/strstr.3.html                  # funcao para verificar se um char* é substr de outro char*
- https://man7.org/linux/man-pages/man1/free.1.html                    # onde encontrar os campos de memoria e como calcular os campos necessarios
- https://tldp.org/HOWTO/pdf/NCURSES-Programming-HOWTO.pdf             # livro de ncurses
- https://man7.org/linux/man-pages/man2/getpagesize.2.html             # como ler um tamanho portavel de pagina (syscall sysconf(_SC_PAGESIZE))
- https://man7.org/linux/man-pages/man0/stdint.h.0p.html               # tipos long, long long, unsigned long... de uma maneira mais concisa
- https://www.gnu.org/software/libc/manual/html_node/Getopt.html       # como fazer o parse do argv 
- https://linux.die.net/man/3/strncpy                                  # copiar char* de forma segura
- https://man7.org/linux/man-pages/man3/localtime.3p.html              # localtime, utilizado em conjunto com time
