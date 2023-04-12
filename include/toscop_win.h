#ifndef __TOSCOP_WIN_HEADER__
#define __TOSCOP_WIN_HEADER__
#include <ncurses.h>
#include <curses.h>
#define MARGIN 1

extern int max_rows;

// macro para printar qualquer coisa com um atributo
#define FORMAT(FUNC, WIN, ATTR, ...) \
    do { wattron(WIN, ATTR); FUNC(WIN, __VA_ARGS__); wattroff(WIN, ATTR); } while(0)

// wrapper da window com suas informacoes basicas
typedef struct t_win {
    WINDOW* win;
    int width;
    int height;
    int x;
    int y;
} t_win;

// enum para alternar entre as window
typedef enum WIN_TABS {
    TH_WIN = 0,
    TP_WIN,
    PROC_WIN,
} WIN_TABS;

/*
 * estrutura para genrenciamento das window do ncurses
 */
typedef struct toscop_wm {
    t_win th_win;           // term_header window (info global)
    t_win tp_win;           // term_procs window  (info da lista de procs)
    t_win proc_win;         // window do proc     (info do processo)

    WIN_TABS c_win;         // window atual que esta com foco
} toscop_wm;

extern toscop_wm* create_toscop_wm(void);
extern void tab_win(toscop_wm* wm);
extern void resize_win(toscop_wm* wm);
extern void draw_wmborders(toscop_wm* wm);
extern void refresh_wm(toscop_wm* wm);
extern void clear_wm(toscop_wm* wm);
extern void wm_free(toscop_wm* wm);

#endif
