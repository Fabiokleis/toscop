#ifndef __TERM_HEADER_GUARD__
#define __TERM_HEADER_GUARD__
#include <sys/sysinfo.h>
#include "w_task.h"
#include "task_list.h"
#define PROC_PATH "/proc/"

/*
 *  cabeçalho do toscop com informações globais
 */
typedef struct term_header {
    struct sysinfo si;       // campo sysinfo contem inumeras informaçoes do sistema
    int t_tasks;             // numero total de pids do /proc
    TaskList* task_list; 
} term_header;

term_header *create_term_header();
void init_tasks(term_header* th);
void tl_print(term_header* th);
void tl_free(term_header* th);

#endif
