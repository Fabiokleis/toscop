#ifndef __TOSCOP_HEADER__
#define __TOSCOP_HEADER__
#include <pthread.h>

// macro para rodar alguma coisa com mutex 
// espera o mutex passado por referencia
#define MUTEX_FUNC(MUTEX, FUNC, ...) do { \
    pthread_mutex_lock(MUTEX); \
    FUNC(__VA_ARGS__); \
    pthread_mutex_unlock(MUTEX); \
} while (0)

#define VA_FUNC(FUNC, ...) do { \
    FUNC(__VA_ARGS__); \
} while(0)

extern pthread_mutex_t toscop_mutex;
extern void run(void);

#endif
