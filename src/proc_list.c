#include <stdlib.h>
#include "../include/proc_list.h"

ProcList* create_proclist(void) {
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

// pega a tail do lista
ProcList* get_lasttl(ProcList* tl) {
    ProcList* aux, *last;

    for (aux = tl; aux != NULL; aux = aux->next)
        last = aux;

    return last;
}

// calcula o numero total de procs iterando a lista toda
unsigned long get_tprocs(ProcList* tl) {
    ProcList* aux = NULL;
    unsigned long tp = 0;
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

void print_proclist(ProcList* tl, int starts_at, int max_rows, toscop_wm* wm) {
    long i = 0;
    ProcList* aux = NULL;


    // percorre n primeiros
    for (aux = tl; i < starts_at && aux != NULL; i++)
        aux = aux->prev;

    // printa cada proc
    for (; i < max_rows && aux != NULL; aux = aux->prev) {

        // para destacar o processo atual
        if (i == starts_at) {
            wattron(wm->tp_win.win, A_REVERSE); // coloca foreground com a cor do background
            print_wproc_line(aux->proc, wm->tp_win);
            wattroff(wm->tp_win.win, A_REVERSE);
            print_wproc_win(aux->proc, wm->proc_win);

        } else {
            print_wproc_line(aux->proc, wm->tp_win);
        }
        i++;

    }
}

void free_proclist(ProcList* tl) {
    ProcList *aux = NULL;
    while (tl != NULL) {
        aux = tl->next;
        proc_free(tl->proc);
        free(tl);
        tl = aux;
    }
}
