#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <pwd.h>
#include <ncurses.h>
#include <stdbool.h>
#include "../include/w_proc.h"
#include "../include/term_header.h"
#include "../include/proc_parser.h"


w_proc* create_w_proc(long pid) { 
    w_proc* proc = malloc(sizeof(w_proc));

    proc->path = malloc(sizeof(char) * 18); // /proc/ + 12
    snprintf(proc->path, 18, PROC_PATH"%ld", pid);  // copia o path para dentro do buffer
                                                    
    // faz o parse do proc
    // caso nao consiga ler retorna false
    if (!stat_proc(proc)) {
        return NULL; // caso ocorra um erro de leitura retorna NULL
    }

    // pega o nome do usuario e outras informacos com base no uid do stat_proc
    struct passwd* r_pwd = getpwuid(proc->uid);
    if (r_pwd == NULL) {
        fprintf(stderr, "ERROR: could not getpwuid of %d %s\n", proc->uid, strerror(errno));
    }
    proc->owner_name = malloc(sizeof(char) * strlen(r_pwd->pw_name) + 1);
    strcpy(proc->owner_name, r_pwd->pw_name); // copia pwd name para onwer_name da proc

    return proc;
}

// faz o parse do /proc/[pid]/stat, man proc para ver os campos
/*
 * 32095 (Isolated Web Co) S 1185 1185 1185 0 -1 4194560 78436 0 0 0 2410 1569 0 0 20 0 27 0 923348 2668761088 47885 18446744073709551615 94866003416480 94866003924768 140731178639232 0 0 0 0 69634 1082133752 0 0 0 17 2 0 0 0 0 0 94866003937056 94866003937160 94866029662208 140731178640931 140731178641165 140731178641165 140731178643423 0
 */
bool stat_proc(w_proc* proc) {
    
    int p_len = strlen(proc->path) + 10;
    char* stat_path = malloc(sizeof(char) * p_len);
    snprintf(stat_path, p_len, "%s/stat", proc->path);
    FILE* stat_file = fopen(stat_path, "r");

    if (stat_file == NULL) {
        fprintf(stderr, "ERROR: could not read %s with fopen: %s\n", stat_path, strerror(errno));
        return false;
    }

    // pega cada campo do /proc/[pid]/stat separados por espaco e coloca em uma estrutura 
    // cada processo tem 52 campos no /proc/[pid]/stat (man proc para ver)
    proc_parse(proc->ptokens, 52, stat_file);

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

    return true;
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
