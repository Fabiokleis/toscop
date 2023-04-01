#ifndef __PROC_LIST_HEADER__
#define __PROC_LIST_HEADER__
#include <stdio.h>
#include <stdlib.h>
#include "w_proc.h"

typedef struct proc_list {
   w_proc *proc;
   struct proc_list *next;
   struct proc_list *prev;
} ProcList;

ProcList* create_proclist();
ProcList* add(ProcList* tl, w_proc* n_proc);
ProcList* get_lasttl(ProcList* tl);
int get_tprocs(ProcList* tl);

void free_proclist(ProcList* tl);
void print_proclist(ProcList *tl, int starts_at, int max_rows);

#endif
