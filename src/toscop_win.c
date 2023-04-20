#include <curses.h>
#include <ncurses.h>
#include <stdlib.h>
#include <assert.h>
#include "toscop.h"
#include "toscop_win.h"

// variaveis de controle de tamanho
static int max_rows = 0;
static int th_height = 0;
static int max_width = 0;
static int tp_height = 0;

// funcoes de inicializacao geral
static void init(void);
static void calc_sizes(void);
static void init_wins(toscop_wm* wm);
static void init_wmcolors(void);
static inline t_win create_t_win(int h, int w, int x, int y);

// funcoes de gerenciamento das window
static void resize_win(toscop_wm* wm);
static void draw_wmborders(toscop_wm* wm);
static void refresh_wm(toscop_wm* wm);
static void clear_wm(toscop_wm* wm);
static t_win* get_cwin(toscop_wm* wm);

// funcoes internas para gerenciar o estado interno de cada window
static void tab_win(toscop_wm* wm);
static void inc_starts_at(uint64_t* st_at);
static void sub_starts_at(uint64_t* st_at);

// cria o gerenciador de window do toscop 
toscop_wm* create_toscop_wm(void) {
    init(); 
    toscop_wm* wm = (toscop_wm *) malloc(sizeof(toscop_wm));
    init_wmcolors();
    init_wins(wm); 
    return wm; 
}
// inicializa a tela do ncurses
static void init(void) {
    initscr();
    timeout(0); // para nao ficar blocando
    raw();
    keypad(stdscr, TRUE); // para capturar as setinhas
    curs_set(FALSE); 
    noecho();
}

// inicializa todas as window do wm
static void init_wins(toscop_wm* wm) {
    calc_sizes();

    // window das informacoes globais do toscop (term_header)
    t_win th_win = create_t_win(th_height, max_width, MARGIN, 0);
   
    // window da lista de processos (term_procs)
    t_win tp_win = create_t_win(
            tp_height,
            max_width,
            MARGIN,
            th_win.height
    ); 

    // window do processo (w_proc)
    t_win w_proc = create_t_win(
            LINES / 5,
            COLS / 4,
            th_win.width + MARGIN, 
            0
    );  

    // window do com informações do filesystem
    t_win fs_win = create_t_win(
            tp_height - (LINES / 3),
            max_width - MARGIN, 
            th_win.width + MARGIN,
            w_proc.height
    );

    // window de cada sessao do toscop
    wm->th_win = th_win;
    wm->tp_win = tp_win;
    wm->proc_win = w_proc;
    wm->fs_win = fs_win;
    wm->c_win = TP_WIN; // seta por padrao como a win da lista de procs com foco
}

// inicializa todos os pares de cores
static void init_wmcolors(void) {
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

    }

}

// calcula variaveis dos tamanhos
static void calc_sizes(void) {
    th_height = (2 * MARGIN + 6);
    max_rows = LINES - th_height - (2 * MARGIN);
    max_width = COLS / 2;
    tp_height = 2 * MARGIN + max_rows;
}

// cria uma wrapper da window do ncurses 
static inline t_win create_t_win(int h, int w, int x, int y) {
    return (t_win){
        .win = newwin(h, w, y, x),
        .height = h,
        .width = w,
        .x = x,
        .y = y,
        .starts_at = 0,
    };
}

// recria as window do wm com novos valores de tamanho
static void resize_win(toscop_wm *wm) {
    clear_wm(wm);
    wm_free(wm);
    init(); // iniciliza stdscr
    init_wins(wm); // inicializa win do wm
    refresh_wm(wm);
}

// desenha as bordas das telas principais
static void draw_wmborders(toscop_wm* wm) {

    box(wm->th_win.win, 0, 0);
    box(wm->tp_win.win, 0, 0);
    box(wm->proc_win.win, 0, 0);
    box(wm->fs_win.win, 0, 0);

    switch (wm->c_win) {
        case TH_WIN: FORMAT(box, wm->th_win.win, COLOR_PAIR(3), 0, 0); break;
        case TP_WIN: FORMAT(box, wm->tp_win.win, COLOR_PAIR(3), 0, 0); break;
        case PROC_WIN: FORMAT(box, wm->proc_win.win, COLOR_PAIR(3), 0, 0); break;
        case FS_WIN: FORMAT(box, wm->fs_win.win, COLOR_PAIR(3), 0, 0); break;
        default:
            break;
    }
}

// escreve nas telas principais do toscop
static void refresh_wm(toscop_wm* wm) {
    wrefresh(wm->th_win.win);
    wrefresh(wm->tp_win.win);
    wrefresh(wm->proc_win.win);
    wrefresh(wm->fs_win.win);
}

// limpa as win principais do toscop
static void clear_wm(toscop_wm *wm) {
    wclear(wm->th_win.win);
    wclear(wm->tp_win.win);
    wclear(wm->proc_win.win);
    wclear(wm->fs_win.win);
}

// limita ate o maximo de procs
static void inc_starts_at(uint64_t* st_at) { 
    (*st_at)++; 
    *st_at = *st_at % total_procs;
}

// limita ate 0, se passar vai para o ultimo proc
static void sub_starts_at(uint64_t* st_at) {
    *st_at = (int64_t) *st_at - 1 < 0 ? total_procs - 1 : *st_at - 1;
} 

// alterna entre cada window
static void tab_win(toscop_wm* wm) {
    wm->c_win = (wm->c_win + 1) % (FS_WIN + 1); // fs_win é a ultima window
}

// retorna a referencia da window com base na variante de win 
static t_win* get_cwin(toscop_wm* wm) {
    t_win* w_def = &wm->tp_win;

    switch (wm->c_win) {
        case PROC_WIN: w_def = &wm->proc_win; break;
        case TH_WIN: w_def = &wm->th_win; break;
        case FS_WIN: w_def = &wm->fs_win; break;
        case TP_WIN: 
        default: 
            break;
    }

    // caso default fica na win da lista de procs
    return w_def;
}

// trata as teclas pressionadas
void handle_key(toscop_wm *wm, int* k_p) {
    *k_p = wgetch(stdscr); // captura na win padrao 

    // para cada tecla faz uma ação
    switch (*k_p) {
        case 9: tab_win(wm); break; // 9 é o codigo da tecla tab
        case KEY_RESIZE: resize_win(wm); break;
        case KEY_DOWN: inc_starts_at(&get_cwin(wm)->starts_at); break;
        case KEY_UP: sub_starts_at(&get_cwin(wm)->starts_at); break;
        default:
            break;
    }
}

// mostra informacoes globais de debug
void show_debug_info(toscop_wm* wm, double refresh_t) {
    uint64_t st = wm->tp_win.starts_at; // caso default fica na win da lista de procs

    // pega o starts_at com base na tela selecionada
    switch (wm->c_win) {
        case PROC_WIN: st = wm->proc_win.starts_at; break;
        case TH_WIN: st = wm->th_win.starts_at; break;
        case FS_WIN: st = wm->fs_win.starts_at; break;
        case TP_WIN: 
        default:
            break;
    }

    mvwprintw(
        stdscr, LINES - 2, 0,
        "\n  debug: %d"
        ",  c_window: %d"
        ",  total_procs: %lu"
        ",  starts_at: %lu"
        ",  refresh_t: %lf\n",
        fdebug, wm->c_win, 
        total_procs, st, refresh_t
    );
    refresh();
}

// mostra todos as window do toscop
// todas informacoes mostradas sao de variaveis globais
void show_toscop(toscop_wm* wm) {
    clear_wm(wm);                                          // limpa todas as win
    th_print(th, wm);                                      // printa o term_header
    tp_print(tp, wm, wm->tp_win.starts_at, total_procs);   // printa o term_procs
    tfs_print(tfs, wm->fs_win);                            // printa o term_fs
    draw_wmborders(wm);                                    // desenha borda
    refresh_wm(wm);                                        // escreve de fato na tela
}

// deleta todas as window criadas
// nao da free do wm
void wm_free(toscop_wm* wm) {
    delwin(wm->th_win.win);
    delwin(wm->tp_win.win);
    delwin(wm->proc_win.win);
    delwin(wm->fs_win.win);
    endwin();
}
