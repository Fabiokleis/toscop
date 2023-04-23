#ifndef __TERM_FS__
#define __TERM_FS__

#include <stdint.h>
#include <sys/statfs.h>
#include "toscop_win.h"
#define GB (double) (1024 * 1024 * 1024)

// wrapper do struct statfs
typedef struct w_fs { 
    char* mnt_point;    // ponto onde esta montado o fs
    char* fs_type;      // tipo de fs (extN, btrfs, dev, devtmpfs, tmpfs, run...)
    double fs_size;     // tamanho total do em GB
    double fs_fsize;    // tamanho total livre em GB
    double fs_used;     // % de uso do fs 
} w_fs;

// lista com todos os fs
typedef struct FsList {
    w_fs* fs;
    struct FsList* next_lfs;
} FsList;

// tabela de fs
typedef struct term_fs {
    FsList* fs_list;
} term_fs;

extern uint64_t total_fs;

extern term_fs* create_term_fs(void); 
extern void tfs_print(term_fs* tfs, t_win fs_win);
extern void tfs_free(term_fs* tfs);

#endif
