#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>
#include "../include/term_procs.h"

// define as variaveis de estado dos procs para o term_header
unsigned long r_procs = 0;
unsigned long s_procs = 0;
unsigned long z_procs = 0;
unsigned long i_procs = 0;
unsigned long total_procs = 0;

// funcao que deve ser utilizada apenas nesse arquivo
static void init_procs(term_procs* tp);

term_procs* create_term_procs(void) {
    term_procs* tp = malloc(sizeof(term_procs));
    init_procs(tp); // le o /proc/[pid] e monta a lista de procs
    return tp;
}

// inicializa todos as procs numa lista duplamente encadeada
static void init_procs(term_procs* tp) {

    unsigned long i = 0;
    DIR* proc_d = opendir("/proc");
    struct dirent* pid_f;

    if (proc_d == NULL) {
        fprintf(stderr, "ERROR: could not read /proc witp opendir: %s\n", strerror(errno));
        exit(1);
    }

    // cria lista duplamente encadeada
    tp->proc_list = create_proclist();
    ProcList* aux = create_proclist();

    // le cada arquivo do /proc que começa com numero, os pids (processos)
    while ((pid_f = readdir(proc_d))) {
        if (!isdigit(*pid_f->d_name))
            continue;

        aux = add(tp->proc_list, create_w_proc(strtol(pid_f->d_name, NULL, 10)));
        if (aux != tp->proc_list) { // pula caso nao tenha conseguido criar um w_proc
            tp->proc_list = aux;
            i++;
        }
    }

    total_procs = i; // total de diretorios lidos
    if (total_procs == 0 || total_procs != get_tprocs(tp->proc_list)) {
        fprintf(stderr, "ERROR: could not load process correctly: %ld\n", total_procs);
        exit(1);
    }
    tp->proc_list_tail = get_lasttl(tp->proc_list); // seta para ser o ultimo

    closedir(proc_d);
}

// limpa a lista de processos e o term_procs
void tp_free(term_procs *tp) {
    //free_proclist(tp->proc_list);
    free_proclist(tp->proc_list_tail);
    free(tp);
}

// printa a lista de procs
void tp_print(term_procs* tp, toscop_wm* wm, int starts_at){
    wattron(wm->tp_win.win, A_BOLD);
    wprintw(wm->tp_win.win, "\n  PID\tUSER PR NI S COMMAND\n");
    wattroff(wm->tp_win.win, A_BOLD);

    print_proclist(tp->proc_list_tail, starts_at, MAX_ROWS + starts_at, wm);
}
