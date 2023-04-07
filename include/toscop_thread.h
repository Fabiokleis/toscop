#ifndef __TOSCOP_THREAD_HEADER__
#define __TOSCOP_THREAD_HEADER__
#include <pthread.h>
#include "term_header.h"
#include "term_procs.h"
#include "toscop_win.h"
#define MAX_TIME (double) 3.0  // refresh a cada 3 secs
#define N_TO_S (double) 1000000000

// estrutura basica de thread do toscop
typedef struct toscop_thread_t {
    pthread_t thread_id;
    term_header* th;                // gerenciador das informacoes globais
    term_procs* tp;                 // gerenciador da lista de procs
    toscop_wm* wm;                  // gerenciador de window para cada info
} toscop_thread_t;

// variaveis globais das threads
extern unsigned long starts_at;
extern int k_p;
extern double refresh_t;

extern void* print_th(void* arg);
extern void* refresh_th(void* arg);

#endif
