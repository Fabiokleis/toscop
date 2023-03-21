#ifndef __TASK_LIST_HEADER__
#define __TASK_LIST_HEADER__
#include <stdio.h>
#include <stdlib.h>
#include "w_task.h"

typedef struct task_list {
   w_task task;
   struct task_list *next;
   struct task_list *prev;
} TaskList;

TaskList* create_tasklist();
TaskList* add(TaskList* tl, w_task n_task);
TaskList* search_task(TaskList* tl, w_task n_task);
TaskList* remove_task(TaskList* tl, w_task n_task);

void free_tasklist(TaskList* tl);
void print_tasklist(TaskList *tl);

#endif
