#include <curses.h>
#include <ncurses.h>
#include "../include/toscop.h"
#include "../include/toscop_thread.h"
#include "../include/toscop_win.h"

int long starts_at = 0;
int k_p = 0;
double refresh_t = 0;

// thread para atualizar a lista de processos
void* refresh_th(void* arg) {
    
    toscop_thread_t* trt = arg;
    struct timespec st = {0}, ct = {0};

    // apenas deixa dar join se for sair do programa
    while (k_p != 'q') {
        clock_gettime(CLOCK_MONOTONIC, &ct); // atualiza a cada iteracao o tempo atual com tepo MONOTÔNICO
        // calcula o tempo atual - tempo do começo
        refresh_t = (((double)(ct.tv_sec - st.tv_sec) * N_TO_S) +
            (double)(ct.tv_nsec - st.tv_nsec)) / N_TO_S; // nanosec por sec

        // caso tenha passado MAX_TIME sec da refresh 
        if (refresh_t >= MAX_TIME) {

            pthread_mutex_lock(&toscop_mutex);

            cpu_stats last_stat = trt->th->cpu_stat; // guarda o ultimo valor de cpu usage

            tl_free(trt->th); // limpa lista de processos anterior
            trt->th = create_term_header(); // cria nova lista
           
            init_cpu_stats(trt->th, last_stat);
            st = ct; // reseta o clock
            
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
                    starts_at = (starts_at % tpt->th->t_procs); // limita ate o numero de procs

                    pthread_mutex_unlock(&toscop_mutex);
            } break;

            case KEY_UP: {
                    pthread_mutex_lock(&toscop_mutex);

                    starts_at = starts_at - 1 < 0 ? tpt->th->t_procs - 1 : starts_at - 1;

                    pthread_mutex_unlock(&toscop_mutex);
            } break;

            default:
                break;
        }

        if (p_t >= 0.2) { // a cada 0.2 segundos é atualizado o print
            pthread_mutex_lock(&toscop_mutex);

            clear_wm(tpt->wm);
            draw_wmborders(tpt->wm);
            tl_print(tpt->th, starts_at, tpt->wm);
            wprintw(tpt->wm->main_win, "\nt_procs: %ld\nstarts_at: %ld\nrefresh_t: %lf\n", tpt->th->t_procs, starts_at, refresh_t);
            pr_t = clock();
            refresh_wm(tpt->wm); // escrever de fato na tela

            pthread_mutex_unlock(&toscop_mutex);

        } 
    }

    pthread_exit(NULL);
}
