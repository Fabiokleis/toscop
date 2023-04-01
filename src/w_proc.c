#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <pwd.h>
#include <ncurses.h>
#include "w_proc.h"
#include "term_header.h"
#include "proc_parser.h"

extern int r_procs;
extern int s_procs;
extern int z_procs;
extern int i_procs;

w_proc* create_w_proc(long int pid) { 
    w_proc* proc = malloc(sizeof(w_proc));

    proc->path = malloc(sizeof(char) * 18); // /proc/ + 12
    snprintf(proc->path, 18, PROC_PATH"%ld", pid);  // copia o path para dentro do buffer
                                                    
    // pega informacoes da proc uid, gid
    stat_proc(proc);

    // pega o nome do usuario e outras informacos com base no uid do stat_proc
    struct passwd* r_pwd;
    r_pwd = getpwuid(proc->uid);
    if (r_pwd == NULL) {
        fprintf(stderr, "ERROR: could not getpwuid of %d %s\n", proc->uid, strerror(errno));
    }
    proc->owner_name = malloc(sizeof(char) * strlen(r_pwd->pw_name) + 1);
    strcpy(proc->owner_name, r_pwd->pw_name); // copia pwd name para onwer_name da proc

    return proc;
}

void stat_proc(w_proc* proc) {
    
    // le o /proc/[pid]/stat
    FILE* stat_file;
    int p_len = strlen(proc->path) + 7;
    char* stat_path = malloc(sizeof(char) * p_len);
    snprintf(stat_path, p_len, "%s/stat", proc->path);

    stat_file = fopen(stat_path, "r");

    if (stat_file == NULL) {
        fprintf(stderr, "ERROR: could not read %s with fopen: %s\n", stat_path, strerror(errno));
        exit(1);
    }

    // pega cada campo do /proc/[pid]/stat separados por espaco e coloca em uma estrutura 
    // cada processo tem 52 campos no /proc/[pid]/stat (man proc para ver)
    proc_parse(proc->ptokens, stat_file);

    // para verificar a quantidade de estados dos processos
    switch (proc->ptokens[2].value[0]) {
        case 'S':
            s_procs++;
            break;
        case 'Z':
            z_procs++;
            break;
        case 'I':
            i_procs++;
            break;
        case 'R':
            r_procs++;
            break;
        default:
            break;
    }

    fclose(stat_file);
    struct stat sb;

    if (stat(proc->path, &sb) == -1) {
        fprintf(stderr, "ERROR: could not stat %s: %s\n", proc->path, strerror(errno));
        exit(1);
    }
    proc->uid = sb.st_uid;

}

void print_wproc(w_proc* proc) {

    printw("\t%s\t%s\t%-18s%s\t%s\t%-24s\t\n", 
            proc->ptokens[0].value, 
            proc->ptokens[2].value, 
            proc->owner_name, 
            proc->ptokens[17].value, 
            proc->ptokens[18].value,
            proc->ptokens[1].value
    );

}

void proc_free(w_proc* proc) {
    free(proc->path);
    free(proc->owner_name);
}
