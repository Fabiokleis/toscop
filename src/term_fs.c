#include <curses.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <mntent.h>
#include <assert.h>
#include <stdbool.h>
#include "term_fs.h"
#include "toscop.h"
#include "toscop_win.h"

uint64_t total_fs = 0;

static void read_fs(term_fs* tfs);
static FsList* add_w_fs(FsList* lfs, w_fs* fs);
static void free_lfs(FsList* lfs);
static void print_w_fs(w_fs* fs, t_win fs_win);
static void print_lfs(FsList* lfs, t_win fs_win);
static void free_w_fs(w_fs* fs);
static void free_lfs(FsList* lfs);

// cria um novo term fs com uma lista de fs do /proc/self/mounts
term_fs* create_term_fs(void) {
    term_fs* tfs = (term_fs *) malloc(sizeof(term_fs));

    read_fs(tfs);

    return tfs;
}

// adiciona na cabeça da lista
static FsList* add_w_fs(FsList* lfs, w_fs* fs) {
    assert(fs != NULL);
    FsList* no = (FsList *) malloc(sizeof(FsList));
    no->fs = fs;
    if (lfs == NULL) {
        no->next_lfs = NULL;
        return no;
    }
    
    no->next_lfs = lfs;

    return no;
}

// le /proc/self/mounts e cria a lista de fs
static void read_fs(term_fs* tfs) {
    // a syscall statfs retorna um struct statfs

    assert(tfs != NULL);
    tfs->fs_list = NULL; // inicializa lfs

    FILE* mnt_file = setmntent("/proc/self/mounts", "r");
    if (mnt_file == NULL) {
        fprintf(stderr, "ERROR: could not read /proc/self/mountinfo with setmntent: %s\n", strerror(errno));
        exit(1);
    }

    // para nao ter que fazer o parse na mao, o mntent retorna cada campo bunitinho
    struct mntent *mnt_ent;
    while ((mnt_ent = getmntent(mnt_file)) != NULL) {
        
        struct statfs st;
        if (statfs(mnt_ent->mnt_dir, &st) == -1) {
            /* apenas mostra que não conseguiu ler um fs
             * caso esteja em debug, pois é muito comum
             * nao conseguir ler por falta de permissão */
            if (fdebug)
                fprintf(stderr, "ERROR: could statfs %s: %s\n", mnt_ent->mnt_dir, strerror(errno));

            // ignora caso nao consiga ler
            continue;
        }
       
        // aceita apenas fs que tem size maior 0
        if (st.f_blocks == 0)
            continue;
       
        // new fs
        w_fs *fs = (w_fs *) malloc(sizeof(w_fs));

        // fs type
        uint64_t g_len = strlen(mnt_ent->mnt_type);
        fs->fs_type = (char *) malloc(g_len + 1);
        strncpy(fs->fs_type, mnt_ent->mnt_type, g_len + 1);

        // fs mount point
        g_len = strlen(mnt_ent->mnt_dir);
        fs->mnt_point = (char *) malloc(g_len + 1);
        strncpy(fs->mnt_point, mnt_ent->mnt_dir, g_len + 1);
        
        // fs size
        fs->fs_size= st.f_bsize * st.f_blocks / GB; // size in GB
        fs->fs_fsize = st.f_bsize * st.f_bfree / GB; // size in GB
        fs->fs_used = (double) (fs->fs_size - fs->fs_fsize) / fs->fs_size * 100.0; // % de uso do fs

        // add to lfs
        tfs->fs_list = add_w_fs(tfs->fs_list, fs);
        total_fs++; // total de filesystem lido
    }

    // para limpar o struct mntent
    endmntent(mnt_file);

}

// printa um unico wrapper de fs
static void print_w_fs(w_fs* fs, t_win fs_win) { 
    wprintw(fs_win.win,
        "  %-8s %8.2lfGb %8.2lfGb %6.2lf%% %-32s\n",
        fs->fs_type,
        fs->fs_size,
        fs->fs_fsize,
        fs->fs_used,
        fs->mnt_point
    );
}

// printa cada fs da lista de w_fs
static void print_lfs(FsList* lfs, t_win fs_win) {

    assert(lfs != NULL);
    FsList* aux = lfs;
    uint64_t i = 0;

    // passa n primeiros
    for (; aux != NULL && i < fs_win.starts_at; ++i)
        aux = aux->next_lfs;

    for (; aux != NULL; aux = aux->next_lfs) {
        // caso for o item selecionado printa com fg e bg invertidos
        if (i == fs_win.starts_at) {
            wattron(fs_win.win, COLOR_PAIR(2) | A_BOLD);
            print_w_fs(aux->fs, fs_win);
            wattroff(fs_win.win, COLOR_PAIR(2) | A_BOLD);
        } else {
            print_w_fs(aux->fs, fs_win);
        }
        i++;
    }
}

// apenas mostra a lista de fs
void tfs_print(term_fs *tfs, t_win fs_win) {
    assert(tfs != NULL);
    assert(tfs->fs_list != NULL);
    assert(fs_win.win != NULL);

    FORMAT(wprintw, fs_win.win, A_BOLD, "\n  TYPE\t\t SIZE\t    FREE    USED MOUNT\n");
    print_lfs(tfs->fs_list, fs_win);
}

static void free_w_fs(w_fs* fs) {
    assert(fs != NULL);
    free(fs->mnt_point);
    free(fs->fs_type);
    free(fs);
}

static void free_lfs(FsList* lfs) {
    assert(lfs != NULL);
    FsList *aux = NULL;
    while (lfs != NULL) {
        free_w_fs(lfs->fs);
        aux = lfs->next_lfs;
        free(lfs);
        lfs = aux;
    }
}

void tfs_free(term_fs* tfs) {
    assert(tfs != NULL);
    free_lfs(tfs->fs_list);
    free(tfs);
}
