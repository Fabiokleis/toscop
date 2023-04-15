#ifndef __TOSCOP_THREAD_HEADER__
#define __TOSCOP_THREAD_HEADER__
#include <pthread.h>
#define N_TO_S (double) 1000000000

// estrutura basica de thread do toscop
typedef struct toscop_thread_t {
    pthread_t thread_id;
} toscop_thread_t;

extern void* print_th(void* arg);
extern void* refresh_th(void* arg);

#endif
