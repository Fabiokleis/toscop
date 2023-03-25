#include "proc_list.h"
#include "w_proc.h"
#include <ncurses.h>

ProcList* create_proclist() {
   return NULL;
}

ProcList* add(ProcList *tl, w_proc* n_proc) {

    if (n_proc != NULL) {
        ProcList *no = (ProcList *) malloc(sizeof(ProcList));
        no->proc = n_proc;
        no->prev = NULL;
        no->next = tl;
        if (tl != NULL)
          tl->prev = no;
        return no;   
    } else {
        return tl;
    }

}

ProcList* get_lasttl(ProcList* tl) {
    ProcList* aux, *last;

    for (aux = tl; aux != NULL; aux = aux->next)
        last = aux;

    return last;
}

int get_tprocs(ProcList* tl) {
    ProcList* aux;
    int tp = 0;
    if (tl->prev == NULL) {
        aux = tl;
        while (aux != NULL) {
            aux = aux->next;
            tp++;
        }
    } else if (tl->next == NULL) {
        aux = get_lasttl(tl);
        while (aux != NULL) {
            aux = aux->prev;
            tp++;
        }
    }
    return tp;
}

void print_proclist(ProcList* tl, int starts_at, int max_rows) {
    long int i = 0;
    ProcList* aux;


    // percorre n primeiros
    for (aux = tl; i < starts_at && aux != NULL; i++)
        aux = aux->prev;

    // printa cada proc
    for (; i < max_rows && aux != NULL; aux = aux->prev) {

        // para destacar o processo atual
        if (i == starts_at) {
            attron(A_REVERSE); // coloca foreground com a cor do background
            print_wproc(aux->proc);
            attroff(A_REVERSE);
        } else {
            print_wproc(aux->proc);
        }
        i++;

    }
}

ProcList* set_lasttl(ProcList *tl, int starts_at) {
    long int i = 0;
    ProcList* aux;
    for (aux = tl; i < starts_at && aux != NULL; i++)
        aux = aux->prev;

    return aux;
}

void free_proclist(ProcList* tl) {
    ProcList *aux;
    while (tl != NULL) {
        aux = tl->next;
        proc_free(tl->proc);
        free(tl);
        tl = aux;
    }
}
