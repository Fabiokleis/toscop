#include "task_list.h"
#include "w_task.h"

TaskList* create_tasklist() {
   return NULL;
}

TaskList* add(TaskList *tl, w_task n_task) {
   TaskList *no = (TaskList *) malloc(sizeof(TaskList));
   no->task = n_task;
   no->prev = NULL;
   no->next = tl;
   if (tl != NULL)
      tl->prev = no;
   return no;   
}

TaskList* search_task(TaskList *tl, w_task n_task) {
   TaskList *aux;
   for (aux = tl; aux != NULL; aux = aux->next) {
      if (aux->task.pid == n_task.pid)	
	 return aux;     
   }
   return NULL;
}

TaskList* remove_task(TaskList* tl, w_task n_task) {
    TaskList *no = search_task(tl, n_task);
    if (no == NULL)
        return tl;
    if (tl == no)
        tl = no->next;
    if (no->next != NULL)
        no->next->prev = no->prev;
    if (no->prev != NULL)
        no->prev->next = no->next;
    free(no);
    return tl;
}

void free_tasklist(TaskList* tl) {
    TaskList *aux;
    while (tl != NULL) {
        aux = tl->next;
        free(tl);
        tl = aux;
    }
}

void print_tasklist(TaskList* tl) {
    if (tl != NULL) {
        // passa todos os nós e vem `retornando` imprimindo
        print_tasklist(tl->next);
        print_wtask(&tl->task);
    }
}
