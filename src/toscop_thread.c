#include <curses.h>
#include <ncurses.h>
#include "../include/toscop.h"
#include "../include/toscop_thread.h"
#include "../include/toscop_win.h"
#include "../include/term_procs.h"

unsigned long starts_at = 0;
int k_p = 0;
double refresh_t = 0;

// thread para atualizar a lista de processos
void* refresh_th(void* arg) {
    
    toscop_thread_t* trt = arg;
    struct timespec st = {0}, ct = {0};
    clock_gettime(CLOCK_MONOTONIC, &st); // pega o tempo deis do boot para o start_time
    cpu_stats last_stat = trt->th->cpu_stat; // guarda o ultimo valor de cpu usage

    // apenas deixa dar join se for sair do programa
    while (k_p != 'q') {

        clock_gettime(CLOCK_MONOTONIC, &ct); // atualiza a cada iteracao o tempo atual com tempo MONOTÔNICO deis do boot

        // calcula o tempo atual - tempo do começo
        refresh_t = (ct.tv_sec - st.tv_sec) + ((ct.tv_nsec - st.tv_nsec) / N_TO_S); // nanosec por sec


        // pega o 
        if (refresh_t == MAX_TIME - 1 && !cpu_stat_equals(trt->th->cpu_stat, last_stat))
            last_stat = trt->th->cpu_stat;

        // caso tenha passado MAX_TIME sec da refresh 
        if (refresh_t >= MAX_TIME) {

            pthread_mutex_lock(&toscop_mutex);

            th_free(trt->th); // limpa informacoes globais anterior
            trt->th = create_term_header(); // cria as novas informacoes
           
            tp_free(trt->tp); // limpa lista de procs anterior
            trt->tp = create_term_procs(); // cria nova lista

            init_cpu_stats(trt->th, last_stat); // calcula delta
            st = ct; // reseta o clock
            
            refresh_t = 0;
            
            pthread_mutex_unlock(&toscop_mutex);
       }

    } 
    pthread_exit(NULL);
}

// thread com loop principal
// thread para printar a lista de processos
void* print_th(void* arg) {

    toscop_thread_t* tpt = arg;

  
    clock_t pr_t = clock();
    double p_t = 0;

    while (k_p != 'q') {
        k_p = wgetch(stdscr); 

        p_t = (double)(clock() - pr_t) / CLOCKS_PER_SEC;

        // para poder navegar entre os processos
        switch (k_p) {
            case KEY_DOWN: {
                    pthread_mutex_lock(&toscop_mutex);

                    starts_at++;
                    starts_at = (starts_at % total_procs); // limita ate o numero de procs

                    pthread_mutex_unlock(&toscop_mutex);
            } break;

            case KEY_UP: {
                    pthread_mutex_lock(&toscop_mutex);

                    starts_at = (long int)starts_at - 1 < 0 ? total_procs - 1 : starts_at - 1;

                    pthread_mutex_unlock(&toscop_mutex);
            } break;

            default:
                break;
        }

        if (p_t >= 0.2) { // a cada 0.2 segundos é atualizado o print
            pthread_mutex_lock(&toscop_mutex);

            clear_wm(tpt->wm);                      // limpa todas as win
            th_print(tpt->th, tpt->wm);             // printa o term_header
            tp_print(tpt->tp, tpt->wm, starts_at);  // printa o term_procs
            wprintw(tpt->wm->proc_win.win, "\n  total_procs: %ld\n  starts_at: %ld\n  refresh_t: %lf\n", total_procs, starts_at, refresh_t);

            draw_wmborders(tpt->wm);                // desenha borda

            pr_t = clock();      // reseta clock
            refresh_wm(tpt->wm); // escrever de fato na tela

            pthread_mutex_unlock(&toscop_mutex);

        } 
    }

    pthread_exit(NULL);
}
