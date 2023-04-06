#ifndef __TOSCOP_THREAD_HEADER__
#define __TOSCOP_THREAD_HEADER__
#include <pthread.h>
#include "term_header.h"
#include "toscop_win.h"
#define MAX_TIME 3.0  // refresh a cada 3 secs
#define N_TO_S 1000000000

// estrutura basica de thread do toscop
typedef struct toscop_thread_t {
    pthread_t thread_id;
    term_header* th;
    toscop_wm* wm;
} toscop_thread_t;

// variaveis globais das threads
extern long starts_at;
extern int k_p;
extern double refresh_t;

void* print_th(void* arg);
void* refresh_th(void* arg);

#endif
