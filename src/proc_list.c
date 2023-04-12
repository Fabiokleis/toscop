#include <curses.h>
#include <assert.h>
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
        return NULL;
    }

}

// pega a tail do lista
ProcList* get_lasttl(ProcList* tl) {
    assert(tl != NULL);
    ProcList* aux, *last;

    for (aux = tl; aux != NULL; aux = aux->next)
        last = aux;

    return last;
}

// calcula o numero total de procs iterando a lista toda
uint64_t get_tprocs(ProcList* tl) {
    assert(tl != NULL);
    ProcList* aux = NULL;
    uint64_t tp = 0;
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

// faz o print esperando a tail da lista, pois fazemos o add na cabeça, logo a tail tem o 1 w_proc
void print_proclist(ProcList* tl, int64_t starts_at, uint32_t max_rows, toscop_wm* wm) {
    int64_t i = 0;
    ProcList* aux = NULL;

    // percorre n primeiros
    for (aux = tl; i < starts_at && aux != NULL; i++)
        aux = aux->prev;

    // printa cada proc
    for ( ; i < max_rows && aux != NULL; aux = aux->prev) {
        // para destacar o processo atual
        if (i == starts_at) {
            wattron(wm->tp_win.win, COLOR_PAIR(2) | A_BOLD);
            print_wproc_line(aux->proc, wm->tp_win);
            wattroff(wm->tp_win.win, COLOR_PAIR(2) | A_BOLD);

            print_wproc_win(aux->proc, wm->proc_win);
        } else {
            print_wproc_line(aux->proc, wm->tp_win);
        }
        i++;

    }
}
// da free na lista e cada proc
void free_proclist(ProcList* tl) {
    ProcList *aux = NULL;
    while (tl != NULL) {
        proc_free(tl->proc);
        aux = tl->next;
        free(tl);
        tl = aux;
    }
}
