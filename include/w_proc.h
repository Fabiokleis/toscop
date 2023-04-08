#ifndef __PROC_HEADER__
#define __PROC_HEADER__
#include "proc_parser.h"
#include "toscop_win.h"

/*
 * conteudo de um processo
 */
typedef struct w_proc {
    token ptokens[52];          // numero de campos do /proc/[pid]/stat
    char* path;                 // caminho absoluto -> /proc/[pid]
    char* owner_name;           // nome do usuario dono da proc
    int uid;                    // user id
    int gid;                    // grupo dono da proc

    unsigned long r_mem;        // (rss) ram total de memoria do processo 
    unsigned long v_mem;        // virtual size total do processo
    unsigned long pr_mem;       // total de paginas de rss do processo
    unsigned long pv_mem;       // total de paginas de vm do processo

} w_proc;

extern w_proc* create_w_proc(unsigned long pid);
extern void print_wproc_line(w_proc* wproc, t_win list_win);
extern void print_wproc_win(w_proc* wproc, t_win proc_win);
extern void proc_free(w_proc* proc);

#endif
