#include <curses.h>
#include <ncurses.h>
#include <stdlib.h>
#include "../include/toscop_win.h"

static void init(void);
static void init_wmcolors(toscop_wm* wm);
static inline t_win create_t_win(int h, int w, int x, int y);

// cria do gerenciador de window do toscop 
toscop_wm* create_toscop_wm() {
    init();
    toscop_wm* wm = malloc(sizeof(toscop_wm));

    // window das informacoes globais do toscop (term_header)
    t_win th_win = create_t_win(TH_HEIGHT, MAX_WIDTH, MARGIN, 0);
   
    // window da lista de processos (term_procs)
    t_win tp_win = create_t_win(TP_HEIGHT, MAX_WIDTH, MARGIN, th_win.height); 

    // window do processo (w_proc)
    t_win w_proc = create_t_win(LINES / 4, COLS / 4, th_win.width + MARGIN, 0);  

    // window de cada sessao do toscop
    wm->th_win = th_win;
    wm->tp_win = tp_win;
    wm->proc_win = w_proc;

    wm->c_win = TH_WIN; // seta por padrao como a win da lista de procs com foco

    init_wmcolors(wm);
    return wm; 
}

static void init_wmcolors(toscop_wm* wm) {
    // caso tenha cores seta as cores do toscop
    if (has_colors() && can_change_color()) {
        start_color(); 
        // o black fica grey as vezes, entao faço init manualmente
        init_color(COLOR_BLACK, 0, 0, 0); 


        // default color
        init_pair(1, COLOR_WHITE, COLOR_BLACK);
        // proc line color
        init_pair(2, COLOR_WHITE, COLOR_CYAN);
        // border color
        init_pair(3, COLOR_CYAN, COLOR_BLACK);


        wbkgd(stdscr, COLOR_PAIR(1));
        wbkgd(wm->th_win.win, COLOR_PAIR(1));
        wbkgd(wm->tp_win.win, COLOR_PAIR(1));
        wbkgd(wm->proc_win.win, COLOR_PAIR(1));
    }

}

static void init(void) {
    // inicializa a tela do ncurses
    initscr();
    timeout(0); // para nao ficar blocando
    raw();
    keypad(stdscr, TRUE); // para capturar as setinhas
    curs_set(FALSE); 
    noecho();
}

// cria uma wrapper da window do ncurses 
static inline t_win create_t_win(int h, int w, int x, int y) {
    return (t_win){
        .height = h,
        .width = w,
        .x = x,
        .y = y,
        .win = newwin(h, w, y, x),
    };
}

// alterna entre as window
void tab_win(toscop_wm* wm) {
    wm->c_win = (wm->c_win + 1) % (PROC_WIN + 1); // proc_win é a ultima window
}

// desenha as bordas das telas principais
void draw_wmborders(toscop_wm* wm) {

    box(wm->th_win.win, 0, 0);
    box(wm->tp_win.win, 0, 0);
    box(wm->proc_win.win, 0, 0);

    switch (wm->c_win) {
        case TH_WIN: {
            FORMAT(box, wm->th_win.win, COLOR_PAIR(3), 0, 0);
        } break;

        case TP_WIN: {
            FORMAT(box, wm->tp_win.win, COLOR_PAIR(3), 0, 0);
        } break;

        case PROC_WIN: {
            FORMAT(box, wm->proc_win.win, COLOR_PAIR(3), 0, 0);
        } break;

        default: {
        } break;
    }
}

// escreve nas telas principais do toscop
void refresh_wm(toscop_wm* wm) {
    wrefresh(wm->th_win.win);
    wrefresh(wm->tp_win.win);
    wrefresh(wm->proc_win.win);
}

// limpa as win principais do toscop
void clear_wm(toscop_wm *wm) {
    wclear(wm->th_win.win);
    wclear(wm->tp_win.win);
    wclear(wm->proc_win.win);
}

// deleta todas as window criadas
void wm_free(toscop_wm* wm) {
    refresh_wm(wm);
    delwin(wm->th_win.win);
    delwin(wm->tp_win.win);
    delwin(wm->proc_win.win);
    endwin();
}

