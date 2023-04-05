#include <bits/time.h>
#include <curses.h>
#include <stdio.h>
#include <ncurses.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "../include/term_header.h"


#define MAX_TIME 5.0  // refresh a cada 3 secs
#define N_TO_S 1000000000

// variaveis globais para gerenciar threads
static pthread_mutex_t toscop_mutex;
static long starts_at = 0;
static int k_p = 0;
static double refresh_t = 0;

// estrutura basica de thread do toscop
typedef struct toscop_thread_t {
    pthread_t thread_id;
    term_header* th;
} toscop_thread_t;


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

            clear(); // limpa tela 

            tl_print(tpt->th, starts_at);
            printw("\nt_procs: %ld\nstarts_at: %ld\nrefresh_t: %lf\n", tpt->th->t_procs, starts_at, refresh_t);
            pr_t = clock();
            refresh(); // escrever de fato na tela

            pthread_mutex_unlock(&toscop_mutex);

        } 
    }

    pthread_exit(NULL);
}

int main() {

    pthread_mutex_init(&toscop_mutex, NULL);
    // inicializa a tela do ncurses
    initscr();
    timeout(0); // para nao ficar blocando
    raw();
    keypad(stdscr, TRUE); // para capturar as setinhas
    curs_set(FALSE); 
    noecho();


    // atributo para dar join
    pthread_attr_t attr;
    int s = pthread_attr_init(&attr);

    if (s != 0) {
        fprintf(stderr, "ERROR: could not create attr with pthread_att_init: %s\n", strerror(errno));
        exit(1);
    }

    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // th tem a lista de processos
    term_header* th = create_term_header();

    // thread para printar os processos
    toscop_thread_t print_thread;
    print_thread.thread_id = 0;
    print_thread.th = th;

    int st = pthread_create(&print_thread.thread_id, &attr, print_th, &print_thread); // cria thread com loop principal

    if (st) {
        fprintf(stderr, "ERROR: could not create print thread with pthread_create: %s\n", strerror(errno));
        exit(1);
    }
    // thread para atualizar o th
    toscop_thread_t refresh_thread;
    refresh_thread.thread_id = 1;
    refresh_thread.th = th; // aponta para o mesmo th

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
    } else {
        printw("deu join na thread de print %c\n", k_p);
        refresh();
        usleep(1000000);
    }

    sj = pthread_join(refresh_thread.thread_id, NULL);
    if (sj < 0) {
        fprintf(stderr, "ERROR: could not join refresh_thread: %s\n", strerror(errno));
        exit(1);
    } else {
        printw("deu join na thread de refresh %c\n", k_p);
        refresh();
        usleep(1000000);
    }

    // free em tudo que foi usado
    pthread_mutex_destroy(&toscop_mutex);
    pthread_attr_destroy(&attr);
    tl_free(th);
    endwin();

    pthread_exit(NULL);
}
