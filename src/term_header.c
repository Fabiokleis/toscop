#include <sys/sysinfo.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "../include/term_header.h"

// funcoes que devem ser utilizadas apenas no term_header 
static void init_time_settings(term_header* th);
static void init_mem_settings(term_header* th);
static void init_loadavg(term_header* th);


// cria e inicializa os campos do th
term_header* create_term_header(void) {
    // zera todos os contadores caso um novo term_header seja criado
    r_procs = 0;
    s_procs = 0;
    z_procs = 0;
    i_procs = 0;
    
    term_header *th = malloc(sizeof(term_header)); 
    sysinfo(&th->si); // inicializa sysinfo que contem varias informacoes do sistema
    th->t_threads = th->si.procs;

    init_time_settings(th); // cria todos campos de tempo/data
    init_mem_settings(th); // adiciona as informacoes de memoria global
    init_cpu_stats(th, (cpu_stats){0}); // adiona as informacoes de cpu 
    init_loadavg(th); // calcula load average


    return th;
}

// compara o struct cpu_stats
bool cpu_stat_equals(cpu_stats stat1, cpu_stats stat2) {
    return stat1.total == stat2.total;
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


// seta todos os campos de memoria provindos do sysinfo
// e do /proc/meminfo
static void init_mem_settings(term_header* th) {

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

    mem_stat.up_mem = ((double) mem_stat.u_mem / mem_stat.t_mem) * 100.0;
    mem_stat.fp_mem = ((double) mem_stat.f_mem / mem_stat.t_mem) * 100.0;

    th->mem_stat = mem_stat;
    fclose(mem_info);

}

// seta todos os campos de hora/data 
static void init_time_settings(term_header* th) {
    
    th->ti = malloc(sizeof(struct tm));

    // time settings
    time_t r_time; 
    time(&r_time); 
    th->ti = localtime(&r_time); // inicializa o timeinfo

    th->h_uptime = th->si.uptime / 3600 % 24; // horas (1h -> 3600 segundos) 24 horas
    th->d_uptime = th->si.uptime / 86400; // dias (1d -> 86400 segundos)
                                           
    // (total de minutos do uptime - (quantidade de minutos de 1 dia x total de dias) - total de horas em minutos)
    th->m_uptime = (th->si.uptime / 60) - (th->d_uptime * 1440) - (th->h_uptime * 60); 
}


// calcula o loadavg com base nos valores do struct sysinfo
static void init_loadavg(term_header *th) {
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


// mostra as informacoes globais: cpu memoria e threads 
void th_print(term_header* th, toscop_wm* wm) {

    wprintw(
        wm->th_win.win,
        "\n  Toscop - %02d:%02d:%02d up %ld days, %ld:%ld, ",
        th->ti->tm_hour, th->ti->tm_min, th->ti->tm_sec,
        th->d_uptime,
        th->h_uptime,
        th->m_uptime
    );
    wprintw(
        wm->th_win.win,
        "load avg: %.2f, %.2f, %.2f\n",
        th->lavg[0], th->lavg[1], th->lavg[2]
    );

    wprintw(
        wm->th_win.win,
        "  Procs: %ld total, Threads: %ld, %ld running, %ld sleeping, %ld zombie, %ld idle\n", 
        total_procs,
        th->t_threads,
        r_procs,
        s_procs,
        z_procs,
        i_procs
    );

    wprintw(
        wm->th_win.win, 
        "  Cpu%%: %.2f%% used, %.2f%% us, %.2f%% sys, %.2f%% idle\n",
        th->cpu_stat.cpu_usage,
        th->cpu_stat.cpu_us, 
        th->cpu_stat.cpu_sys,
        th->cpu_stat.cpu_idle
    );

    wprintw(
        wm->th_win.win,
        "  Mem%%: %.2f%% used, %2.f%% free, VirtMem MB: %lu total\n", 
        th->mem_stat.up_mem, 
        th->mem_stat.fp_mem,
        th->mem_stat.t_vm
    );

    wprintw(
        wm->th_win.win,
        "  Mem MB: %lu total, %lu used, %lu free, %lu shared\n", 
        th->mem_stat.t_mem,
        th->mem_stat.u_mem, 
        th->mem_stat.f_mem,
        th->mem_stat.s_mem
    );

    wprintw(
        wm->th_win.win,
        "  Swap: %lu total, %lu free, %lu used, %lu buffer/cache\n", 
        th->mem_stat.t_swap, 
        th->mem_stat.f_swap,
        th->mem_stat.t_swap - th->mem_stat.f_swap,
        th->mem_stat.b_mem + th->mem_stat.c_mem 
    );
}

// free no term_header
void th_free(term_header* th) {
    free(th);
}
