#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include "../include/term_procs.h"

// define as variaveis de estado dos procs para o term_header
uint64_t r_procs = 0;
uint64_t s_procs = 0;
uint64_t z_procs = 0;
uint64_t i_procs = 0;
uint64_t total_procs = 0;

// funcao que deve ser utilizada apenas nesse arquivo
static void init_procs(term_procs* tp);

term_procs* create_term_procs(void) {
    // zera todos os contadores caso um novo term_procs seja criado
    r_procs = 0;
    s_procs = 0;
    z_procs = 0;
    i_procs = 0;

    term_procs* tp = malloc(sizeof(term_procs));
    init_procs(tp); // le o /proc/[pid] e monta a lista de procs
    return tp;
}

// inicializa todos as procs numa lista duplamente encadeada
static void init_procs(term_procs* tp) {

    uint64_t i = 0;
    DIR* proc_d = opendir("/proc");
    struct dirent* pid_f;

    if (proc_d == NULL) {
        fprintf(stderr, "ERROR: could not open /proc with opendir: %s\n", strerror(errno));
        exit(1);
    }

    // cria lista duplamente encadeada
    tp->proc_list = create_proclist();      // NULL
    tp->proc_list_tail = create_proclist(); // NULL
    ProcList* aux = create_proclist();      // NULL


    errno = 0; // para diferenciar quando ocorrer um erro no readdir
    // le cada arquivo do /proc que começa com numero, os pids (processos)
    while ((pid_f = readdir(proc_d)) != NULL) {
        if (!isdigit(*pid_f->d_name))
            continue;

        uint64_t pid = strtoul(pid_f->d_name, NULL, 10);

        // caso o falhe o strtoul, voltamos o errno para 0 e continuamos o loop
        if (pid == 0) {
            errno = 0;
            continue;
        }

        // pula caso nao tenha conseguido criar um w_proc
        aux = add(tp->proc_list, create_w_proc(pid));
        if (aux != tp->proc_list) {
            i++;
            tp->proc_list = aux;
        }
    }

    // caso algum erro deu no readdir, o errno é modificado
    if (pid_f == NULL && errno != 0) {
        fprintf(stderr, "ERROR: could not read %s with readdir: %s\n", "/proc", strerror(errno));
        exit(1);
    }


    total_procs = i; // total de diretorios lidos
    if (total_procs != get_tprocs(tp->proc_list)) {
        fprintf(stderr, "ERROR: could not load process correctly: %ld\n", total_procs);
        closedir(proc_d);
        exit(1);
    }
    tp->proc_list_tail = get_lasttl(tp->proc_list); // seta para ser o ultimo

    closedir(proc_d);
}

// limpa a lista de processos e o term_procs
void tp_free(term_procs *tp) {
    free_proclist(tp->proc_list);
    free(tp);
}

// printa a lista de procs
void tp_print(term_procs* tp, toscop_wm* wm, uint64_t starts_at){
    assert(tp->proc_list_tail != NULL);
    FORMAT(wprintw, wm->tp_win.win, A_BOLD,"\n     PID USER       PR  NI S COMMAND\n");
    print_proclist(tp->proc_list_tail, starts_at, (uint64_t)max_rows + starts_at, wm);
}
