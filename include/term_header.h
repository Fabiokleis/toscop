#ifndef __TERM_HEADER__
#define __TERM_HEADER__
#include <linux/sysinfo.h>
#include <stdint.h>
#include "toscop_win.h"

#define PROC_PATH "/proc/"
#define LINESZ 1024
#define MB (uint64_t) (1024*1024) // bytes to MB
#define UNIT (uint64_t) 1024      // bytes to kB

// estrutura para guardar os valores de tempo de uso em sec e % de uso do cpu
typedef struct cpu_stats {
    uint64_t t_user;              // total de secs do us (us + ni)
    uint64_t t_system;            // total de secs do sy (sy + hi + si)
    uint64_t t_idle;              // total de secs do id (tempo ocioso)
    uint64_t total;               // soma de todos secs

    double cpu_usage;             // total % cpu 
    double cpu_us;                // total % us
    double cpu_sys;               // total % sy 
    double cpu_idle;              // total % id
} cpu_stats;

// estrutura para guardar os valores de memoria
typedef struct mem_stats {
    uint64_t t_mem;            // mem total                         
    uint64_t f_mem;            // mem free 
    uint64_t b_mem;            // mem alocada por buffers
    uint64_t s_mem;            // mem compartilhada
    uint64_t t_swap;           // swap total              
    uint64_t f_swap;           // swap free               
    uint64_t t_vm;             // vm total
    uint64_t c_mem;            // cached memory
    uint64_t u_mem;            // used memory

    double fp_mem;             // % de mem free
    double up_mem;             // % de mem usada
} mem_stats;

/*
 *  cabeçalho do toscop com informações globais
 */
typedef struct term_header {
    struct sysinfo si;            // campo sysinfo contem inumeras informaçoes do sistema
    struct tm* ti;                // campo timeinfo contem informacoes de tempo
    cpu_stats cpu_stat;           // valores de tempo de uso do cpu, em secs e em %
    mem_stats mem_stat;           // informacoes de memoria obtidos via sysinfo, e feito os calculos de %
    double lavg[3];               // loadavg do sistema
    uint64_t d_uptime;            // tempo em sec dos dias
    uint64_t h_uptime;            // tempo em sec das horas
    uint64_t m_uptime;            // tempo em sec dos minutos
    uint64_t t_procs;             // total de processos
    uint64_t t_threads;           // total de threads de todos os processos (/proc/[pid]/task/*)
} term_header;


// variaveis globais para controlar o numero de processos em cada estado
extern uint64_t r_procs;     // running
extern uint64_t s_procs;     // sleeping
extern uint64_t z_procs;     // zoombie
extern uint64_t i_procs;     // idle
extern uint64_t total_procs; // numero total de procs

extern term_header* create_term_header(void);
extern void calc_cpu_stats(term_header* th, cpu_stats c_stats);
extern cpu_stats get_real_cpu_stats(void);
extern void th_print(term_header* th, toscop_wm* wm);
extern void th_free(term_header* th);

#endif
