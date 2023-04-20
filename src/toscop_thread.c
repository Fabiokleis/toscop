#define _GNU_SOURCE
#include <stdlib.h>
#include <unistd.h>
#include "toscop.h"
#include "toscop_thread.h"

static int k_p = 0;
static double refresh_t = 0;

// thread para atualizar a lista de processos 
void* refresh_th(void* arg) {
    (void) arg; // parametro nao utilizado
    
    struct timespec st = {0}, ct = {0};
    clock_gettime(CLOCK_MONOTONIC, &st); // pega o tempo deis do boot para o start_time
    cpu_stats last_stat = {0}; 

    // apenas deixa dar join se for sair do programa
    while (k_p != 'q') {

        clock_gettime(CLOCK_MONOTONIC, &ct); // atualiza a cada iteracao o tempo atual com tempo MONOTÔNICO deis do boot

        // calcula o tempo atual - tempo do começo
        refresh_t = (ct.tv_sec - st.tv_sec) + ((ct.tv_nsec - st.tv_nsec) / N_TO_S); // nanosec por sec

        // caso tenha passado max_time sec da refresh 
        if (refresh_t >= max_time) {
            st = ct;                  // reseta o clock, start time recebe current time
            last_stat = th->cpu_stat; // guarda o ultimo valor de cpu usage sem delta
      
            pthread_mutex_lock(&toscop_mutex);

            tp_free(tp);              // limpa lista de procs anterior
            tp = create_term_procs(); // cria nova lista
          
            th_free(th);                    // limpa informacoes globais anterior
            th = create_term_header();      // cria o novo term header
            calc_cpu_stats(th, last_stat);  // calcula delta de % cpu

            tfs_free(tfs);              // limpa o fs anterior
            tfs = create_term_fs();     // cria fs novo
           
            pthread_mutex_unlock(&toscop_mutex);
        
        }
    } 
    pthread_exit(NULL);
}

// thread para printar a lista de processos
void* print_th(void* arg) {
    (void) arg; // parametro nao utilizado

    clock_t pr_t = clock();
    double p_t = 0;

    while (k_p != 'q') {

        // muda o estado interdo do wm com base no k_p
        MUTEX_FUNC(&toscop_mutex, handle_key, wm, &k_p);

        p_t = (double)(clock() - pr_t) / CLOCKS_PER_SEC;

        if (p_t >= 0.2) { // a cada 0.2 segundos é atualizado o print
       
            // mostra todas as win do wm
            MUTEX_FUNC(&toscop_mutex, show_toscop, wm);
         
            // mostra informacoes de debug (configurado via flag -v)
            if (fdebug)
                MUTEX_FUNC(&toscop_mutex, show_debug_info, wm, refresh_t);

            pr_t = clock();  // reseta clock
        } 
    }

    pthread_exit(NULL);
}
