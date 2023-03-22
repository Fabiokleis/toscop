#include <curses.h>
#include <stdio.h>
#include <time.h>
#include "term_header.h"
#include <ncurses.h>


int main() {

    term_header* th = create_term_header();    
    

    //int n_rows = LINES;
    //int n_cols = COLS;
    initscr();
    raw();
    keypad(stdscr, TRUE);
    curs_set(FALSE);
    noecho();
   
    int k_p;
    while (k_p != 'q') {
        refresh();

        tl_print(th);

        k_p = wgetch(stdscr);
       
        switch (k_p) {
            case KEY_DOWN:
                  printw("todo\n");
                  break;
                  
            case KEY_UP:
                  printw("todo\n");
                  break;

        }
    }


    endwin();
    
    tl_free(th);

    return 0;
}
