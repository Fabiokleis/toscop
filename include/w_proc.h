#ifndef __PROC_HEADER_GUARD__
#define __PROC_HEADER_GUARD__
#include <sys/sysinfo.h>
#define LINESZ 1024

#include "proc_parser.h"

/*
 * conteudo de um processo
 */
typedef struct w_proc {
    token ptokens[52]; // numero de campos do /proc/[pid]/stat
    char* path;          // caminho absoluto -> /proc/[pid]
    char* owner_name;    // nome do usuario dono da proc
    int uid;             // user id
    int gid;             // grupo dono da proc
} w_proc;

w_proc* create_w_proc(long int pid);
void stat_proc(w_proc* proc);
void print_wproc(w_proc* wproc);
void proc_free(w_proc* proc);

#endif
