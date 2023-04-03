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
#include "../include/term_header.h"
#include "../include/w_proc.h"
#include "../include/proc_list.h"

unsigned long r_procs = 0;
unsigned long s_procs = 0;
unsigned long z_procs = 0;
unsigned long i_procs = 0;

// cria e inicializa os campos do th
term_header* create_term_header() {
    // zera todos os contadores caso um novo term_header seja criado
    r_procs = 0;
    s_procs = 0;
    z_procs = 0;
    i_procs = 0;
    
    term_header *th = malloc(sizeof(term_header)); 
    sysinfo(&th->si); // inicializa sysinfo que contem varias informacoes do sistema
    getloadavg(th->lavg, 3); // carrega load avg no vertor lavg

    init_time_settings(th); // cria todos campos de tempo/data
    init_mem_settings(th); // adiciona as informacoes de memoria global
    init_cpu_stats(th, (cpu_stats){0}); // adiona as informacoes de cpu 
    init_procs(th); // adiciona todas as procs


    return th;
}

// seta todos os campos de memoria provindos do sysinfo
void init_mem_settings(term_header* th) {
    // mem settings
    double mb = 1024 * 1024;
    th->t_mem = th->si.totalram / mb;   // mem total 
    th->f_mem = th->si.freeram / mb;    // mem free 
    th->b_mem = th->si.bufferram / mb;  // mem alocada em por buffers
    th->s_mem = th->si.sharedram / mb;  // mem compartilhada
    th->t_swap = th->si.totalswap / mb; // swap total 
    th->f_swap = th->si.freeswap / mb;  // swap free
}

// seta todos os campos de hora/data 
void init_time_settings(term_header* th) {
    
    th->ti = malloc(sizeof(struct tm));

    // time settings
    time_t r_time; 
    time(&r_time); 
    th->ti = localtime(&r_time); // inicializa o timeinfo

    th->h_uptime = th->si.uptime / 3600; // horas (1h -> 3600 segundos)
    th->d_uptime = th->si.uptime / 86400; // dias (1d -> 86400 segundos)
                                           
    // (total de minutos do uptime - (quantidade de minutos de 1 dia x total de dias) - total de horas em minutos)
    th->m_uptime = (th->si.uptime / 60) - (th->d_uptime * 1440) - (th->h_uptime * 60); 
}

// inicializa todos as procs numa lista duplamente encadeada
void init_procs(term_header* th) {

    long int i = 0;
    DIR* proc_d = opendir("/proc");
    struct dirent* pid_f;

    if (proc_d == NULL) {
        fprintf(stderr, "ERROR: could not read /proc with opendir: %s\n", strerror(errno));
        exit(1);
    }

    // cria lista duplamente encadeada
    th->proc_list = create_proclist(); 

    // le cada arquivo do /proc que começa com numero, os pids (processos)
    while ((pid_f = readdir(proc_d))) {
        if (!isdigit(*pid_f->d_name))
            continue;

        th->proc_list = add(th->proc_list, create_w_proc(strtol(pid_f->d_name, NULL, 10)));
        i++;
    }

    th->t_procs = i; // total de diretorios lidos
    if (th->t_procs == 0) {
        fprintf(stderr, "ERROR: could not load any process: %ld\n", th->t_procs);
        exit(1);
    }

    th->tail = get_lasttl(th->proc_list); // seta para ser o ultimo
    closedir(proc_d);
}

// faz o parse do /proc/stat e guarda numa estrutura,
// man proc para ver os campos, e calcula os valores de uso do cpu 
/* 
 * cpu  209917 419 128259 18553662 18180 5 8314 0 0 0
 *
 */

void init_cpu_stats(term_header *th, cpu_stats c_stats) {

    FILE* stat_file = fopen(PROC_PATH"/stat", "r");
    if (stat_file == NULL) {
        fprintf(stderr, "ERROR: could not read %s with fopen: %s\n", PROC_PATH"/stat", strerror(errno));
        exit(1);
    }

    proc_parse(th->ktokens, 11, stat_file);

    // man top - 2b. TASK and CPU States

    // secs cpu
    long user = strtol(th->ktokens[1].value, NULL, 10) + 
        strtol(th->ktokens[2].value, NULL, 10);

    long system = strtol(th->ktokens[3].value, NULL, 10) +
        strtol(th->ktokens[6].value, NULL, 10) +
        strtol(th->ktokens[7].value, NULL, 10);

    double idle = strtod(th->ktokens[4].value, NULL);
    double t = (double) (user + system + idle); 

    // total secs
    th->cpu_stat.t_user = user - c_stats.t_user;
    th->cpu_stat.t_system = system - c_stats.t_system;
    th->cpu_stat.t_idle = idle - c_stats.t_idle;
    th->cpu_stat.total = t - c_stats.total;

    // % cpu 
    th->cpu_stat.cpu_idle = (double) th->cpu_stat.t_idle / th->cpu_stat.total * 100.0;
    th->cpu_stat.cpu_usage = ((double) th->cpu_stat.total - th->cpu_stat.t_idle) / th->cpu_stat.total * 100.0;
    th->cpu_stat.cpu_sys = (double) th->cpu_stat.t_system / th->cpu_stat.total * 100.0;
    th->cpu_stat.cpu_us = (double) th->cpu_stat.t_user / th->cpu_stat.total * 100.0;


    fclose(stat_file);
}

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
    printw("toscop - %02d:%02d:%02d up %ld days, %ld hours and %ld minutes\n", 
            th->ti->tm_hour, th->ti->tm_min, th->ti->tm_sec,
            th->d_uptime,
            th->h_uptime,
            th->m_uptime
    );

    printw("Tasks: %lu total, %lu running, %lu sleeping, %lu zombie, %lu idle\n", 
            th->t_procs,
            r_procs,
            s_procs,
            z_procs,
            i_procs
    );

    printw("CPU: %.2f%%, %.2f%% us, %.2f%% sys, %.2f%% id, ", 
            th->cpu_stat.cpu_usage, 
            th->cpu_stat.cpu_us, 
            th->cpu_stat.cpu_sys,
            th->cpu_stat.cpu_idle
    );

    printw("load avg: %.2f, %.2f, %.2f\n", th->lavg[0], th->lavg[1], th->lavg[2]);


    printw("MiB Mem: %.1f total, %.1f free, %.1f buffer ram, %.1f ram shared\n", 
            th->t_mem, 
            th->f_mem,
            th->b_mem, 
            th->s_mem
    );

    printw("MiB Swap: %.1f total, %.1f free, %.1f used\n", 
            th->t_swap, 
            th->f_swap,
            th->t_swap - th->f_swap
    );
    printw("\n");
    printw("\tPID\tSTATE\tUSER%-14sPR\tNI\tCOMMAND\n", "");

    // da print em cada proc
    print_proclist(th->tail, starts_at, MAX_ROWS + starts_at);
}

// free no term_header
void tl_free(term_header* th) {
    free_proclist(th->proc_list); 
    free(th);
}
