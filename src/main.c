#include <curses.h>
#include <stdio.h>
#include <ncurses.h>
#include <time.h>
#include <pthread.h>
#include "term_header.h"



int main() {
    
    term_header* th = create_term_header();

    int starts_at, k_p = 0;

    initscr();
    raw();
    timeout(0);
    keypad(stdscr, TRUE);
    curs_set(FALSE);
    noecho();
    getch();
     
    clock_t start_t = clock();
    double refresh_t = 0;

    while (k_p != 'q') {
        refresh_t = (double)(clock() - start_t) / CLOCKS_PER_SEC;
        if (refresh_t >= 5.0) {

            tl_free(th); // limpa lista de processos anterior
            th = create_term_header(); // cria nova lista
            start_t = clock(); // reseta o clock
        }
        tl_print(th, starts_at);


        k_p = wgetch(stdscr); // pega a tecla
       
        switch (k_p) {
            case KEY_DOWN:
                starts_at = starts_at + 1 == th->t_procs ? 0 : starts_at + 1;
                break;
                  
            case KEY_UP:
                starts_at = starts_at - 1 < 0 ? th->t_procs - 1 : starts_at - 1;
                break;

        }
        refresh();
        clear();

    }

    endwin();
    tl_free(th);

    return 0;
}
