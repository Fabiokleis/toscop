#ifndef __PROC_HEADER__
#define __PROC_HEADER__
#include "proc_parser.h"
#include "toscop_win.h"
#include <stdint.h>

/*
 * conteudo de um processo
 */
typedef struct w_proc {
    token ptokens[52];          // numero de campos do /proc/[pid]/stat
    char* path;                 // caminho absoluto -> /proc/[pid]
    char* owner_name;           // nome do usuario dono da proc
    int uid;                    // user id
    int gid;                    // grupo dono da proc

    uint64_t r_mem;        // (rss) ram total de memoria do processo 
    uint64_t v_mem;        // virtual size total do processo
    uint64_t pr_mem;       // total de paginas de rss do processo
    uint64_t pv_mem;       // total de paginas de vm do processo

    uint64_t stack_pages;  // total de paginas da stack
    uint64_t stack_size;   // total em kB da stack

    uint64_t heap_pages;   // total de paginas da heap
    uint64_t heap_size;    // total em kB da heap

    uint64_t text_pages;   // total de paginas do text
    uint64_t text_size;    // total em kB do text
} w_proc;

extern w_proc* create_w_proc(uint64_t pid);
extern void print_wproc_line(w_proc* wproc, t_win list_win);
extern void print_wproc_win(w_proc* wproc, t_win proc_win);
extern void proc_free(w_proc* proc);

#endif
