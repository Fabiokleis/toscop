#include <stdio.h>
#include <ncurses.h>
#include "term_header.h"


int main() {
    term_header* th = create_term_header();

    int starts_at, k_p = 0;

    initscr();
    raw();
    keypad(stdscr, TRUE);
    curs_set(FALSE);
    noecho();

 
    while (k_p != 'q') {

        clear();
        refresh();

        tl_print(th, starts_at);

        k_p = wgetch(stdscr); // pega a tecla
       
        switch (k_p) {
            case KEY_DOWN:
                starts_at = starts_at + 1 == th->t_procs - 1 ? 0 : starts_at + 1;
                break;
                  
            case KEY_UP:
                starts_at = starts_at - 1 <= 0 ? th->t_procs - 1 : starts_at - 1;
                break;

        }
    }


    endwin();
    tl_free(th);

    return 0;
}
