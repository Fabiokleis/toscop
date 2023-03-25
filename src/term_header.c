#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <ncurses.h>
#include "term_header.h"
#include "w_proc.h"
#include "proc_list.h"


// cria e inicializa os campos do th
term_header* create_term_header() {
    term_header *th = malloc(sizeof(term_header)); 
    th->ti = malloc(sizeof(struct tm));
    th->proc_list = create_proclist();

    sysinfo(&th->si); // inicializa sysinfo

    // time settings
    time_t r_time; 
    time(&r_time); 
    th->ti = localtime(&r_time); // inicializa o timeinfo

    th->d_uptime = th->si.uptime / 86400; // dias (1d -> 86400 segundos)
    th->h_uptime = (th->si.uptime / 3600); // horas (1h -> 3600 segundos)
                                           //
    // (total de minutos do uptime - (quantidade de minutos de 1 dia x total de dias) - total de horas em minutos)
    th->m_uptime = (th->si.uptime / 60) - (th->d_uptime * 1440) - (th->h_uptime * 60); 

    // mem settings
    double mb = 1024 * 1024;
    th->t_mem = th->si.totalram / mb;
    th->f_mem = th->si.freeram / mb;
    th->b_mem = th->si.bufferram / mb;
    th->s_mem = th->si.sharedram / mb;
    th->t_swap = th->si.totalswap / mb;
    th->f_swap = th->si.freeswap / mb;


    //parse_cpu_stats(th); TODO: parse all cpu stats
    init_procs(th); // adiciona todas as procs


    return th;
}

// inicializa todos as procs numa lista duplamente encadeada
void init_procs(term_header* th) {

    int i = 0;
    DIR* proc_d = opendir("/proc");
    struct dirent* pid_f;

    
    if (proc_d == NULL) {
        fprintf(stderr, "ERROR: could not read /proc with opendir: %s\n", strerror(errno));
        exit(1);
    }
   
    // le cada arquivo do /proc que começa com numero, os pids (processos)
    while ((pid_f = readdir(proc_d))) {
        if (!isdigit(*pid_f->d_name))
            continue;

        th->proc_list = add(th->proc_list, create_w_proc(strtol(pid_f->d_name, NULL, 10)));
        i++;
    }

    th->t_procs = i;
    if (th->t_procs == 0) {
        fprintf(stderr, "ERROR: could not load any process: %s\n", strerror(errno));
        exit(1);
    }
    closedir(proc_d);

    th->proc_list = get_lasttl(th->proc_list); // seta para ser o ultimo

}

/*
void parse_cpu_stats(term_header *th) {
    FILE* cpu_stat_file;
    char line[LINESZ];

    cpu_stat_file = fopen(PROC_PATH"stat", "r");

    if (cpu_stat_file == NULL) {
        fprintf(stderr, "ERROR: could not read /proc/stat with fopen: %s\n", strerror(errno));
        exit(1);
    }

    while (fgets(line, LINESZ, cpu_stat_file) != NULL) {
        for (int i = 0; line[i] != '\0'; i++) {
            printf("%c", line[i]);
        }
        printf("\n");
    }

    fclose(cpu_stat_file);
}
*/
/* top format
top - 17:31:20 up  1:34,  1 user,  load average: 0.34, 0.35, 0.34
Tasks: 346 total,   1 running, 345 sleeping,   0 stopped,   0 zombie
%Cpu(s):  1.4 us,  0.9 sy,  0.0 ni, 97.7 id,  0.0 wa,  0.0 hi,  0.0 si,  0.0 st
MiB Mem :  15418.0 total,   6862.2 free,   4765.0 used,   3790.8 buff/cache
MiB Swap:  16384.0 total,  16384.0 free,      0.0 used.  10249.2 avail Mem
*/


// mostra as informacoes globais e
// chama o print da proc_list
void tl_print(term_header* th, int starts_at) {
    printw("toscop - %02d:%02d:%02d up %d days, %d hours and %d minutes\n", 
            th->ti->tm_hour, th->ti->tm_min,
            th->ti->tm_sec, th->d_uptime,
            th->h_uptime,
            th->m_uptime);

    printw("Tasks: %d total, procs: %d, ", th->si.procs, th->t_procs);
    printw("load average: %ld, %ld, %ld\n", th->si.loads[0], th->si.loads[1], th->si.loads[2]);


    printw("MiB Mem: %.1f total, %.1f free, %.1f buffer ram, %.1f ram shared\n", 
            th->t_mem, 
            th->f_mem,
            th->b_mem, 
            th->s_mem);

    printw("MiB Swap: %.1f total, %.1f free, %.1f used\n", 
            th->t_swap, 
            th->f_swap,
            th->t_swap - th->f_swap
            );
    printw("\n");
    printw("\tPID\tSTATE\tUSER%-12sPR\tNI\tCOMMAND\n", "");

    // da print em cada proc
    print_proclist(th->proc_list, starts_at, MAX_ROWS + starts_at);
}

// free no term_header
void tl_free(term_header* th) {
    free_proclist(th->proc_list); 
    free(th);
}
