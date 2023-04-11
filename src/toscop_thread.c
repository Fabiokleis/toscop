#define _GNU_SOURCE
#include "../include/toscop.h"
#include "../include/toscop_thread.h"
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
// variaveis globais das threads que sao inicializadas aqui
int k_p = 0;
double refresh_t = 0;
uint64_t starts_at = 0;

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
            st = ct; // reseta o clock, start time recebe current time

            last_stat = th->cpu_stat; // guarda o ultimo valor de cpu usage sem delta
      
            pthread_mutex_lock(&toscop_mutex);

            tp_free(tp); // limpa lista de procs anterior
            tp = create_term_procs(); // cria nova lista
          
            th_free(th); // limpa informacoes globais anterior
            th = create_term_header(); // cria o novo term header
            calc_cpu_stats(th, last_stat); // calcula delta de % cpu
           
            pthread_mutex_unlock(&toscop_mutex);
        
        }
    } 
    pthread_exit(NULL);
}

// limita ate o maximo de procs
static void inc_starts_at(uint64_t* st_at) { 
    (*st_at)++; 
    *st_at = *st_at % total_procs;
}

// limita ate 0, se passar vai para o ultimo proc
static void sub_starts_at(uint64_t* st_at) {
    *st_at = (int64_t) *st_at - 1 < 0 ? total_procs - 1 : *st_at - 1;
}

// thread com loop principal
// thread para printar a lista de processos
void* print_th(void* arg) {
    (void) arg; // parametro nao utilizado

    clock_t pr_t = clock();
    double p_t = 0;

    while (k_p != 'q') {
        k_p = wgetch(stdscr); 

        p_t = (double)(clock() - pr_t) / CLOCKS_PER_SEC;

        // para poder navegar entre os processos
        switch (k_p) {

            case 9: { // 9 é o codigo da tecla tab
                MUTEX_FUNC(&toscop_mutex, tab_win, wm);
            } break;

            case KEY_RESIZE: {
                MUTEX_FUNC(&toscop_mutex, resize_win, wm);
            } break;

            case KEY_DOWN: {
                MUTEX_FUNC(&toscop_mutex, inc_starts_at, &starts_at);
            } break;

            case KEY_UP: {
                MUTEX_FUNC(&toscop_mutex, sub_starts_at, &starts_at);
            } break;

            default:
                break;
        }

        if (p_t >= 0.2) { // a cada 0.2 segundos é atualizado o print
            pthread_mutex_lock(&toscop_mutex);

            clear_wm(wm);                      // limpa todas as win
            th_print(th, wm);                  // printa o term_header
            tp_print(tp, wm, starts_at);       // printa o term_procs
            draw_wmborders(wm);                // desenha borda
            pr_t = clock();                    // reseta clock
            refresh_wm(wm);                    // escrever de fato na tela
          
            // mostra informacoes de debug (configurado via flag -v)
            if (fdebug) {
                mvwprintw(stdscr,
                        LINES - 2, 0, "\n  debug: %d,  c_window: %d,  total_procs: %lu,  starts_at: %lu,  refresh_t: %lf\n",
                        fdebug, wm->c_win, total_procs, starts_at, refresh_t);
                refresh();
            }
            pthread_mutex_unlock(&toscop_mutex);

        } 
    }

    pthread_exit(NULL);
}
