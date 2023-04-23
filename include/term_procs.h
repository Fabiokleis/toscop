#ifndef __TERM_PROCS_HEADER__
#define __TERM_PROCS_HEADER__
#include "proc_list.h"

/*
 * estrutra que guarda todos os processos
 * da lista duplamente encadeada proc_list.
 */
typedef struct term_procs {
    ProcList *proc_list;
    ProcList *proc_list_tail;

} term_procs;

extern term_procs* create_term_procs(void);
extern void tp_print(term_procs* tp, toscop_wm* wm);
extern void tp_free(term_procs* tp);


#endif
