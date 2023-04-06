#ifndef __TOSCOP_WIN_HEADER__
#define __TOSCOP_WIN_HEADER__

#include <ncurses.h>
/*
 * estrutura para genrenciamento das window do ncurses
 */
typedef struct {
    WINDOW* main_win;
    WINDOW* proc_win;
} toscop_wm;

void init(void);
toscop_wm* create_toscop_wm(void);
void draw_wmborders(toscop_wm* wm);
void refresh_wm(toscop_wm* wm);
void clear_wm(toscop_wm* wm);
void wm_free(toscop_wm* wm);

#endif
