#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "../include/toscop.h"
#include "../include/term_header.h"
#include "../include/toscop_thread.h"
#include "../include/toscop_win.h"

pthread_mutex_t toscop_mutex;

// inicializa os structs necessarios, screens, e cria as threads do toscop
// faz o loop principal, o join e limpa todos os recursos utilizados
void run(void) {
    // inicializa ncurses e as window do toscop
    toscop_wm* wm = create_toscop_wm(); 

    // inicializa o unico mutex das threads
    pthread_mutex_init(&toscop_mutex, NULL);
    pthread_attr_t attr;     // atributo para dar join

    int s = pthread_attr_init(&attr);

    if (s != 0) {
        fprintf(stderr, "ERROR: could not create attr with pthread_att_init: %s\n", strerror(errno));
        exit(1);
    }

    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // th tem as informacoes globais
    term_header* th = create_term_header();

    // tp tem a lista de processos
    term_procs* tp = create_term_procs();

    // thread para printar os processos
    toscop_thread_t print_thread;
    print_thread.thread_id = 0;
    print_thread.th = th;
    print_thread.tp = tp;
    print_thread.wm = wm;

    int st = pthread_create(&print_thread.thread_id, &attr, print_th, &print_thread); // cria thread com loop principal

    if (st) {
        fprintf(stderr, "ERROR: could not create print thread with pthread_create: %s\n", strerror(errno));
        exit(1);
    }
    // thread para atualizar o th
    toscop_thread_t refresh_thread;
    refresh_thread.thread_id = 1;
    refresh_thread.th = th; // aponta para o mesmo th
    refresh_thread.tp = tp; // aponta para o mesmo tp
    refresh_thread.wm = NULL;

    // inicializa a thread que faz o refresh da lista de processos
    st = pthread_create(&refresh_thread.thread_id, &attr, refresh_th, &refresh_thread);
    if (st) {
        fprintf(stderr, "ERROR: could not create refresh thread with pthread_create: %s\n", strerror(errno));
        exit(1);
    }

    // join das threads
    int sj = pthread_join(print_thread.thread_id, NULL);
    if (sj < 0) {
        fprintf(stderr, "ERROR: could not join print_thread: %s\n", strerror(errno));
        exit(1);
    } 

    sj = pthread_join(refresh_thread.thread_id, NULL);
    if (sj < 0) {
        fprintf(stderr, "ERROR: could not join refresh_thread: %s\n", strerror(errno));
        exit(1);
    } 
    // free em tudo que foi usado
    pthread_mutex_destroy(&toscop_mutex);
    pthread_attr_destroy(&attr);
    th_free(th); // limpa infos globais
    tp_free(tp); // limpa lista de procs
    wm_free(wm); // mata as window
    pthread_exit(NULL);
}
