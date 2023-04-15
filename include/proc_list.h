#ifndef __PROC_LIST_HEADER__
#define __PROC_LIST_HEADER__
#include <stdint.h>
#include "w_proc.h"
#include "toscop_win.h"

/*
 * lista duplamente encadeada para guardar a struct do proc
 */
typedef struct proc_list {
   w_proc *proc;
   struct proc_list *next;
   struct proc_list *prev;
} ProcList;

extern ProcList* create_proclist(void);
extern ProcList* add(ProcList* tl, w_proc* n_proc);
extern ProcList* get_lasttl(ProcList* tl);
extern uint64_t get_tprocs(ProcList* tl);
extern void free_proclist(ProcList* tl);
extern void print_proclist(ProcList *tl, int64_t starts_at, uint32_t max_rows, toscop_wm* wm);

#endif
