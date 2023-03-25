#ifndef __PROC_HEADER_GUARD__
#define __PROC_HEADER_GUARD__
#include <sys/sysinfo.h>

#define PROC_PATH "/proc/"

/*
 * conteudo de um processo
 */
typedef struct w_proc {
    int pid;           // numero em -> /proc/pid
    char* path;        // caminho absoluto -> /proc/pid
    char* owner_name;  // nome do usuario dono da proc
    int uid;           // user id
    int gid;           // grupo dono da proc
    long int prio;          // prioridade da proc
    char state;        // estado da proc
    int ppid;          // pid do proc pai
    char *comm;        // nome do executavel da proc
    long int nice;          // nice value
    long int nt;            // numero de threads
    unsigned long vt_size;       // memoria virtual
} w_proc;

w_proc* create_w_proc(long int pid);
int stat_proc(w_proc* proc);
void print_wproc(w_proc* wproc);
void proc_free(w_proc* proc);

#endif
