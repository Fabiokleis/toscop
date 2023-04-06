#include <stdlib.h>
#include <ncurses.h>
#include "../include/toscop_win.h"

void init(void) {
    // inicializa a tela do ncurses
    initscr();
    timeout(0); // para nao ficar blocando
    raw();
    keypad(stdscr, TRUE); // para capturar as setinhas
    curs_set(FALSE); 
    noecho();
}

toscop_wm* create_toscop_wm(void) {
    toscop_wm* wm = malloc(sizeof(toscop_wm));
    // window principal do toscop
    int t_w = COLS / 2;
    WINDOW* toscop_win = newwin(LINES, COLS / 2, 1, 1);

    // window do processo
    int p_h = LINES / 4;
    int p_w = COLS / 4;
    int p_x = t_w + 1;
    int p_y = 1;
    WINDOW* proc_win = newwin(p_h, p_w, p_y, p_x);

    //box(toscop_win, 0, 0);
    box(proc_win, 0, 0);
    wrefresh(toscop_win);
    wrefresh(proc_win);

    wm->main_win = toscop_win;
    wm->proc_win = proc_win;

    return wm; 
}

void draw_wmborders(toscop_wm* wm) {
    //box(wm->main_win, 0, 0);
    box(wm->proc_win, 0, 0);
}

void refresh_wm(toscop_wm* wm) {
    wrefresh(wm->main_win);
    wrefresh(wm->proc_win);
}

void clear_wm(toscop_wm *wm) {
    wclear(wm->main_win);
    wclear(wm->proc_win);
}

void wm_free(toscop_wm* wm) {
    refresh_wm(wm);
    delwin(wm->proc_win);
    delwin(wm->main_win);
    endwin();
}
