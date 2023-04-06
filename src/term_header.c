#include <curses.h>
#include <linux/sysinfo.h>
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
term_header* create_term_header(void) {
    // zera todos os contadores caso um novo term_header seja criado
    r_procs = 0;
    s_procs = 0;
    z_procs = 0;
    i_procs = 0;
    
    term_header *th = malloc(sizeof(term_header)); 
    sysinfo(&th->si); // inicializa sysinfo que contem varias informacoes do sistema


    init_time_settings(th); // cria todos campos de tempo/data
    init_mem_settings(th); // adiciona as informacoes de memoria global
    init_cpu_stats(th, (cpu_stats){0}); // adiona as informacoes de cpu 
    init_loadavg(th); // calcula load average
    init_procs(th); // adiciona todas as procs


    return th;
}

// seta todos os campos de memoria provindos do sysinfo
// e do /proc/meminfo
void init_mem_settings(term_header* th) {

    FILE* mem_info = fopen(PROC_PATH"/meminfo", "r");
    if (mem_info == NULL) {
        fprintf(stderr, "ERROR: could not read %s with fopen: %s\n", PROC_PATH"/meminfo", strerror(errno));
        exit(1);
    }

    long t_m = strtol(find_token("MemTotal:", mem_info).value, NULL, 10);
    long b_m = strtol(find_token("Buffers:", mem_info).value, NULL, 10);
    long c_m = strtol(find_token("Cached:", mem_info).value, NULL, 10);
    long vmsz = strtol(find_token("VmallocTotal:", mem_info).value, NULL, 10);

    // mem settings
    // sysinfo guarda em bytes
    // /proc/meminfo guarda em kB
    mem_stats mem_stat = {
        .f_mem = th->si.freeram / MB,
        .s_mem = th->si.sharedram / MB,
        .t_swap = th->si.totalswap / MB, 
        .f_swap = th->si.freeswap / MB,
        .t_mem = t_m / UNIT,   
        .t_vm = vmsz / UNIT,
        .b_mem = b_m / UNIT,
        .c_mem = c_m / UNIT,
    };

    // man free para ver como é feito o calculo
    mem_stat.u_mem = mem_stat.t_mem - mem_stat.f_mem - mem_stat.b_mem - mem_stat.c_mem;

    mem_stat.up_mem = mem_stat.u_mem / mem_stat.t_mem * 100.0;
    mem_stat.fp_mem = mem_stat.f_mem / mem_stat.t_mem * 100.0;

    th->mem_stat = mem_stat;
    fclose(mem_info);

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

    long i = 0;
    DIR* proc_d = opendir("/proc");
    struct dirent* pid_f;

    if (proc_d == NULL) {
        fprintf(stderr, "ERROR: could not read /proc with opendir: %s\n", strerror(errno));
        exit(1);
    }

    // cria lista duplamente encadeada
    th->proc_list = create_proclist();
    ProcList* aux = create_proclist();

    // le cada arquivo do /proc que começa com numero, os pids (processos)
    while ((pid_f = readdir(proc_d))) {
        if (!isdigit(*pid_f->d_name))
            continue;

        aux = add(th->proc_list, create_w_proc(strtol(pid_f->d_name, NULL, 10)));
        if (aux != th->proc_list) {
            th->proc_list = aux;
            i++;
        }
    }

    th->t_procs = i; // total de diretorios lidos
    if (th->t_procs == 0 || th->t_procs != get_tprocs(th->proc_list)) {
        fprintf(stderr, "ERROR: could not load process correctly: %ld\n", th->t_procs);
        exit(1);
    }

    // obtem numero de threads
    th->t_threads = th->si.procs; 

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

    // idle
    double idle = strtod(th->ktokens[4].value, NULL);
    double t = (double) (user + system + idle); 

    th->cpu_stat.t_user = user - c_stats.t_user;
    th->cpu_stat.t_system = system - c_stats.t_system;
    th->cpu_stat.t_idle = idle - c_stats.t_idle;
    th->cpu_stat.total = t - c_stats.total;   // total secs


    // % cpu 
    th->cpu_stat.cpu_idle = (double) th->cpu_stat.t_idle / th->cpu_stat.total * 100.0;
    th->cpu_stat.cpu_usage = ((double) th->cpu_stat.total - th->cpu_stat.t_idle) / th->cpu_stat.total * 100.0;
    th->cpu_stat.cpu_sys = (double) th->cpu_stat.t_system / th->cpu_stat.total * 100.0;
    th->cpu_stat.cpu_us = (double) th->cpu_stat.t_user / th->cpu_stat.total * 100.0;


    fclose(stat_file);
}

// calcula o loadavg com base nos valores do struct sysinfo
void init_loadavg(term_header *th) {
    // veja o /usr/include/linux/sysinfo.h para entender como calcular
    th->lavg[0] = th->si.loads[0] / (double) (1 << SI_LOAD_SHIFT);
    th->lavg[1] = th->si.loads[1] / (double) (1 << SI_LOAD_SHIFT);
    th->lavg[2] = th->si.loads[2] / (double) (1 << SI_LOAD_SHIFT);
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
void tl_print(term_header* th, int starts_at, toscop_wm* wm) {
    wprintw(wm->main_win, "toscop - %02d:%02d:%02d up %ld days, %ld hours and %ld minutes\n", 
            th->ti->tm_hour, th->ti->tm_min, th->ti->tm_sec,
            th->d_uptime,
            th->h_uptime,
            th->m_uptime
    );

    wprintw(wm->main_win, "Procs: %ld total, Threads: %ld, %ld running, %ld sleeping, %ld zombie, %ld idle\n", 
            th->t_procs,
            th->t_threads,
            r_procs,
            s_procs,
            z_procs,
            i_procs
    );

    wprintw(wm->main_win, "CPU: %.2f%% used, %.2f%% us, %.2f%% sys, %.2f%% id, ", 
            th->cpu_stat.cpu_usage,
            th->cpu_stat.cpu_us, 
            th->cpu_stat.cpu_sys,
            th->cpu_stat.cpu_idle
    );

    wprintw(wm->main_win, "load avg: %.2f, %.2f, %.2f\n", th->lavg[0], th->lavg[1], th->lavg[2]);


    wprintw(wm->main_win, "Mem: %.2f total, %.1f used, %.1f free, %.1f shared\n", 
            th->mem_stat.t_mem,
            th->mem_stat.u_mem, 
            th->mem_stat.f_mem,
            th->mem_stat.s_mem
    );

    wprintw(wm->main_win, "Mem: %.2f%% used, %2.f%% free, VirtMem: %.1f total\n", 
            th->mem_stat.up_mem, 
            th->mem_stat.fp_mem,
            th->mem_stat.t_vm
    );

    wprintw(wm->main_win, "Swap: %.1f total, %.1f free, %.1f used, %.1f buffer/cache\n", 
            th->mem_stat.t_swap, 
            th->mem_stat.f_swap,
            th->mem_stat.t_swap - th->mem_stat.f_swap,
            th->mem_stat.b_mem + th->mem_stat.c_mem 
    );
    wprintw(wm->main_win,"\n");
    wprintw(wm->main_win, "  PID\tUSER PR NI S COMMAND\n");

    // da print em cada proc
    print_proclist(th->tail, starts_at, MAX_ROWS + starts_at, wm);
}

// free no term_header
void tl_free(term_header* th) {
    free_proclist(th->proc_list); 
    free(th);
}
