#ifndef __TASK_HEADER_GUARD__
#define __TASK_HEADER_GUARD__
#include <sys/sysinfo.h>

#define PROC_PATH "/proc/"

/*
 * conteudo de uma task
 */
typedef struct w_task {
    long int pid;           // numero em -> /proc/pid
    char* path;             // caminho absoluto -> /proc/pid
    long int uid;           // usuario dono da task
    char* owner_name;       // nome do usuario dono da task
    long int gid;           // grupo dono da task
    long int prio;          // prioridade da task
} w_task;

w_task* create_w_task(long int t_pid);
void stat_task(w_task* task);
void print_wtask(w_task* wtask);

#endif
