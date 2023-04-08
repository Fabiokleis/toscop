#ifndef __TERM_HEADER__
#define __TERM_HEADER__
#include <linux/sysinfo.h>
#include "toscop_win.h"
#include "proc_parser.h"

#define PROC_PATH "/proc/"
#define LINESZ 1024
#define MB (long) (1024*1024) // bytes to MB
#define UNIT (long) 1024      // kB to MB

// estrutura para guardar os valores de tempo de uso em sec e % de uso do cpu
typedef struct cpu_stats {
    long t_user;              // total de secs do us (us + ni)
    long t_system;            // total de secs do sy (sy + hi + si)
    long t_idle;              // total de secs do id (tempo ocioso)
    long total;               // soma de todos secs

    double cpu_usage;         // total % cpu 
    double cpu_us;            // total % us
    double cpu_sys;           // total % sy 
    double cpu_idle;          // total % id
} cpu_stats;

// estrutura para guardar os valores de memoria
typedef struct mem_stats {
    unsigned long t_mem;            // mem total                         
    unsigned long f_mem;            // mem free 
    unsigned long b_mem;            // mem alocada por buffers
    unsigned long s_mem;            // mem compartilhada
    unsigned long t_swap;           // swap total              
    unsigned long f_swap;           // swap free               
    unsigned long t_vm;             // vm total
    unsigned long c_mem;            // cached memory
    unsigned long u_mem;            // used memory

    double fp_mem;           // % de mem free
    double up_mem;           // % de mem usada
} mem_stats;

/*
 *  cabeçalho do toscop com informações globais
 */
typedef struct term_header {
    struct sysinfo si;        // campo sysinfo contem inumeras informaçoes do sistema
    struct tm* ti;            // campo timeinfo contem informacoes de tempo
    token ktokens[11];        // primeira linha do /proc/stat, contendo informacoes do cpu
    cpu_stats cpu_stat;       // valores de tempo de uso do cpu, em secs e em %
    mem_stats mem_stat;       // informacoes de memoria obtidos via sysinfo, e feito os calculos de %
    double lavg[3];           // loadavg do sistema
    long d_uptime;            // tempo em sec dos dias
    long h_uptime;            // tempo em sec das horas
    long m_uptime;            // tempo em sec dos minutos
    long t_procs;             // total de processos
    long t_threads;           // total de threads de todos os processos (/proc/[pid]/task/*)
} term_header;


// variaveis globais para controlar o numero de processos em cada estado
extern unsigned long r_procs;     // running
extern unsigned long s_procs;     // sleeping
extern unsigned long z_procs;     // zoombie
extern unsigned long i_procs;     // idle
extern unsigned long total_procs; // numero total de procs

extern term_header* create_term_header(void);
extern void init_cpu_stats(term_header* th, cpu_stats c_stats);
extern bool cpu_stat_equals(cpu_stats stat1, cpu_stats stat2);
extern void th_print(term_header* th, toscop_wm* wm);
extern void th_free(term_header* th);

#endif
