#ifndef __TERM_HEADER_GUARD__
#define __TERM_HEADER_GUARD__
#include <sys/sysinfo.h>
#include <time.h>
#include "w_task.h"
#include "task_list.h"
#define PROC_PATH "/proc/"
#define LINESZ 1024

/*
 *  cabeçalho do toscop com informações globais
 */
typedef struct term_header {
    struct sysinfo si;       // campo sysinfo contem inumeras informaçoes do sistema
    struct tm* ti;           // campo timeinfo contem informacoes de tempo
    int d_uptime;
    int h_uptime;
    int m_uptime;
    double t_mem;
    double f_mem;
    double b_mem;
    double s_mem;
    double t_swap;
    double f_swap;

    int t_procs;             // total de processos

    TaskList* task_list;
} term_header;


term_header *create_term_header();
void init_tasks(term_header* th);
void parse_cpu_stats(term_header* th);
void tl_print(term_header* th);
void tl_free(term_header* th);

#endif
