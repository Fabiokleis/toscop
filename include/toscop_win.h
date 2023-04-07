#ifndef __TOSCOP_WIN_HEADER__
#define __TOSCOP_WIN_HEADER__
#include <ncurses.h>

#define MAX_ROWS 20
#define MAX_WIDTH (COLS / 2 - 13)
#define MARGIN 1
#define TH_HEIGHT (2 * MARGIN + 6)
#define TP_HEIGHT (2 * MARGIN + MAX_ROWS)

// wrapper da windows com suas informacoes basicas
typedef struct t_win {
    WINDOW* win;
    int width;
    int height;
    int x;
    int y;
} t_win;
/*
 * estrutura para genrenciamento das window do ncurses
 */
typedef struct toscop_wm {
    t_win th_win;           // term_header window (info global)
    t_win tp_win;           // term_procs window  (info da lista de procs)
    t_win proc_win;         // window do proc     (info do processo)
} toscop_wm;


extern toscop_wm* create_toscop_wm(void);
extern void draw_wmborders(toscop_wm* wm);
extern void refresh_wm(toscop_wm* wm);
extern void clear_wm(toscop_wm* wm);
extern void wm_free(toscop_wm* wm);

#endif
