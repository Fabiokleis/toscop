#ifndef __TOSCOP_HEADER__
#define __TOSCOP_HEADER__
#include <pthread.h>
#include <stdbool.h>
// macro para rodar alguma coisa com mutex 
// espera o mutex passado por referencia
#define MUTEX_FUNC(MUTEX, FUNC, ...) do { \
    pthread_mutex_lock(MUTEX); \
    FUNC(__VA_ARGS__); \
    pthread_mutex_unlock(MUTEX); \
} while (0)

extern pthread_mutex_t toscop_mutex; // mutex global
extern double max_time; // tempo maximo para cada refresh
extern bool fdebug; // flag de debug
extern void run(void);
extern void cli(int argc, char** argv); 

#endif
