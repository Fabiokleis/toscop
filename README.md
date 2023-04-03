# toscop
uma versão tosca do comando top
trabalho para disciplina de sistemas operacionais

# refs
- https://wiki.inf.ufpr.br/maziero/lib/exe/fetch.php?media=socm:socm-livro.pdf 
- https://man7.org/linux/man-pages/man2/syscalls.2.html                # lista com todas as syscalls
- https://man7.org/linux/man-pages/man2/sysinfo.2.html                 # struct para pegar informacoes gerais do sistema (memoria, swap...)
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
- https://man7.org/linux/man-pages/man3/getloadavg.3.html              # funcao para obter o loadavg do sistema (sysinfo retorna em long)
- https://man7.org/linux/man-pages/man1/top.1.html                     # para entender como é calculado os valores de uso do cpu
