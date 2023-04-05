#ifndef __TERM_HEADER_GUARD__
#define __TERM_HEADER_GUARD__
#include <sys/sysinfo.h>
#include <time.h>
#include "w_proc.h"
#include "proc_list.h"
#include "proc_parser.h"
#define PROC_PATH "/proc/"
#define LINESZ 1024
#define MAX_ROWS 25
#define MB (double) (1024*1024) // bytes to MB
#define UNIT (double) 1024      // kB to MB

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
    double t_mem;            // mem total                         
    double f_mem;            // mem free 
    double b_mem;            // mem alocada por buffers
    double s_mem;            // mem compartilhada
    double t_swap;           // swap total              
    double f_swap;           // swap free               
    double t_vm;             // vm total
    double c_mem;            // cached memory
    double u_mem;            // used memory

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
    long d_uptime;
    long h_uptime;
    long m_uptime;
    long t_procs;              // total de processos
    long t_threads;            // total de threads de todos os processos (/proc/[pid]/task/*)

    ProcList* proc_list;
    ProcList* tail;
} term_header;


// variaveis globais para controlar o numero de processos em cada estado
extern unsigned long r_procs; // running
extern unsigned long s_procs; // sleeping
extern unsigned long z_procs; // zoombie
extern unsigned long i_procs; // idle


term_header* create_term_header();
void init_time_settings(term_header* th);
void init_mem_settings(term_header* th);
void init_procs(term_header* th);
void init_cpu_stats(term_header* th, cpu_stats c_stats);
void init_loadavg(term_header* th);
void tl_print(term_header* th, int starts_at);
void tl_free(term_header* th);

#endif
