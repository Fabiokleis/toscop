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

// estrutura para guardar os valores de tempo de uso em sec e % de uso do cpu
typedef struct cpu_stats {
    long t_user;              // total de secs do us (us + ni)
    long t_system;            // total de secs do sy (sy + hi + si)
    long t_idle;              // total de secs do id
    long total;               // soma de todos secs

    double cpu_usage;         // total % cpu 
    double cpu_us;            // total % us
    double cpu_sys;           // total % sy 
    double cpu_idle;          // total % id
} cpu_stats;

/*
 *  cabeçalho do toscop com informações globais
 */
typedef struct term_header {
    struct sysinfo si;        // campo sysinfo contem inumeras informaçoes do sistema
    struct tm* ti;            // campo timeinfo contem informacoes de tempo
    token ktokens[11];        // primeira linha do /proc/stat, contendo informacoes do cpu
    cpu_stats cpu_stat;       // valores de tempo de uso do cpu, em secs e em %
    long int d_uptime;
    long int h_uptime;
    long int m_uptime;
    double t_mem;
    double f_mem;
    double b_mem;
    double s_mem;
    double t_swap;
    double f_swap;
    double lavg[3];            // loadavg do sistema
    long int t_procs;          // total de processos

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
void tl_print(term_header* th, int starts_at);
void tl_free(term_header* th);

#endif
