#include <curses.h>
#include <stdio.h>
#include <ncurses.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "term_header.h"

#define MAX_TIME 5

static clock_t g_start_t;
static int starts_at = 0;
static int k_p = 0;

typedef struct term_print_t {
    pthread_t thread_id;
    term_header* th;
    double refresh_t;
} term_print_t;

typedef struct term_refresh_t {
    pthread_t thread_id;
    term_header* th;
} term_refresh_t;



// thread para atualizar a lista de processos
void* refresh_th(void* arg) {
    term_refresh_t* trt = arg;

    tl_free(trt->th); // limpa lista de processos anterior
    trt->th = create_term_header(); // cria nova lista
    g_start_t = clock(); // reseta o clock

    pthread_exit(NULL);
}


// thread com loop principal
// thread para printar a lista de processos
void* print_th(void* arg) {

    term_print_t* tpt = arg;

    // cria estrutura basica para atualizar os processos
    // thread para atualizar o th
    term_refresh_t refresh_thread;
    // inicializa a thread que faz o refresh da lista de processos
    refresh_thread.thread_id = 1;
    refresh_thread.th = tpt->th; // aponta para o mesmo th

    // atributo para dar join
    pthread_attr_t attr;
    int s = pthread_attr_init(&attr);

    if (s != 0) {
        fprintf(stderr, "ERROR: could not create attr with pthread_att_init: %s\n", strerror(errno));
        pthread_exit((void *) -1);
    }

    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    clock_t pr_t = clock();
    double p_t = 0;

    while (k_p != 'q') {
        k_p = wgetch(stdscr); 

        p_t = (double)(clock() - pr_t) / CLOCKS_PER_SEC;

        // para poder navegar entre os processos
        switch (k_p) {
            case KEY_DOWN:
                starts_at = starts_at + 1 == tpt->th->t_procs ? 0 : starts_at + 1;
                break;
            case KEY_UP:
                starts_at = starts_at - 1 < 0 ? tpt->th->t_procs - 1 : starts_at - 1;
                break;
        }

        if (p_t >= 0.1) { // a cada 0.1 segundos é atualizado o print
            clear();

            tl_print(tpt->th, starts_at);
            printw("\nt_procs: %d\nstarts_at: %d\n\n", tpt->th->t_procs, starts_at);
            pr_t = clock();

            refresh(); // escrever de fato na tela

        }

        tpt->refresh_t = (double)(clock() - g_start_t) / CLOCKS_PER_SEC;
        // caso tenha passado 5s cria uma thread para dar refresh 
        if (tpt->refresh_t >= MAX_TIME) {
            int st = pthread_create(&refresh_thread.thread_id, &attr, refresh_th, &refresh_thread);
            if (st) {
                fprintf(stderr, "ERROR: could not create refresh thread with pthread_create: %s\n", strerror(errno));
                pthread_exit((void *) -1); // para no join da main sabermos se deu erro
            }
            int sj = pthread_join(refresh_thread.thread_id, NULL);
            if (sj < 0) {
                fprintf(stderr, "ERROR: could not join refresh_thread: %s\n", strerror(errno));
                pthread_exit((void *) -1); // para no join da main sabermos se deu erro
            } 
        }

    }


    pthread_exit(NULL);
}

int main() {

    initscr();
    timeout(0);
    raw();
    keypad(stdscr, TRUE);
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

    term_header* th = create_term_header();

    // thread para printar os processos
    term_print_t print_thread;

    print_thread.thread_id = 0;
    print_thread.th = th;
    print_thread.refresh_t = 0;

    int st = pthread_create(&print_thread.thread_id, &attr, print_th, &print_thread); // cria thread com loop principal

    if (st) {
        fprintf(stderr, "ERROR: could not create print thread with pthread_create: %s\n", strerror(errno));
        exit(1);
    }

    int sj = pthread_join(print_thread.thread_id, NULL);
    if (sj < 0) {
        fprintf(stderr, "ERROR: could not join print_thread: %s\n", strerror(errno));
        exit(1);
    } else {
        printw("deu join na thread de print %c\n", k_p);
        refresh();
        usleep(1000000);
    }


    // free em tudo que foi usado
    pthread_attr_destroy(&attr);
    tl_free(th);
    endwin();

    pthread_exit(NULL);
}
