#include <curses.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../include/w_proc.h"
#include "../include/term_header.h"

static void init_mem(w_proc* proc);
static bool stat_proc(w_proc* proc);

w_proc* create_w_proc(uint64_t pid) { 
    w_proc* proc = malloc(sizeof(w_proc));

    proc->path = malloc(sizeof(char) * 18); // /proc/ + 12
    snprintf(proc->path, 18, PROC_PATH"%ld", pid);  // copia o path para dentro do buffer
                                                    
    // faz o parse do proc
    // caso nao consiga ler retorna false
    if (!stat_proc(proc)) {
        return NULL; // caso ocorra um erro de leitura retorna NULL
    }
    init_mem(proc); // inicializa todos os campos de memoria (rss, vmsize, heap, stack, text)

    // pega o nome do usuario e outras informacos com base no uid do stat_proc
    struct passwd* r_pwd = getpwuid(proc->uid);
    if (r_pwd == NULL) {
        fprintf(stderr, "ERROR: could not getpwuid of %d %s\n", proc->uid, strerror(errno));
    }
    proc->owner_name = malloc(sizeof(char) * strlen(r_pwd->pw_name) + 1);
    strcpy(proc->owner_name, r_pwd->pw_name); // copia pwd name para onwer_name da proc

    return proc;
}

// calcula o numero de pagina de cada campo de memoria e seu tamanho em MB
static void init_mem(w_proc* proc) {

    // sysconf(_SC_PAGESIZE) deve retornar o tamanho de uma pagina (padrao 4096 bytes)

    proc->pv_mem = strtoul(proc->ptokens[22].value, NULL, 10) / sysconf(_SC_PAGESIZE); // vmsize em paginas do proc
    proc->v_mem = proc->pv_mem * sysconf(_SC_PAGESIZE) / MB; // vmsize total em MB do proc
    proc->pr_mem = strtol(proc->ptokens[23].value, NULL, 10); // rss total de paginas do proc
    proc->r_mem = proc->pr_mem * sysconf(_SC_PAGESIZE) / MB;  // rss total em MB do proc
    
    // man proc (/proc/[pid]/status)
    int p_len = strlen(proc->path) + 10;
    char* status_path = malloc(sizeof(char) * p_len);
    snprintf(status_path, p_len, "%s/status", proc->path);
    FILE *proc_status = fopen(status_path, "r");

    // stack, heap e text

    proc->heap_size = strtoul(find_token("VmData:", proc_status).value, NULL, 10); // size em kB
    proc->heap_pages = proc->heap_size * 1024 / sysconf(_SC_PAGESIZE);

    proc->stack_size = strtoul(find_token("VmStk:", proc_status).value, NULL, 10); // size em kB
    proc->stack_pages = proc->stack_size * 1024 / sysconf(_SC_PAGESIZE);

    proc->text_size = strtoul(find_token("VmExe:", proc_status).value, NULL, 10); // size me kB
    proc->text_pages = proc->text_size * 1024 / sysconf(_SC_PAGESIZE);

        
    fclose(proc_status);
    free(status_path);
}

// faz o parse do /proc/[pid]/stat, man proc para ver os campos
/*
 * 32095 (Isolated Web Co) S 1185 1185 1185 0 -1 4194560 78436 0 0 0 2410 1569 0 0 20 0 27 0 923348 2668761088 47885 18446744073709551615 94866003416480 94866003924768 140731178639232 0 0 0 0 69634 1082133752 0 0 0 17 2 0 0 0 0 0 94866003937056 94866003937160 94866029662208 140731178640931 140731178641165 140731178641165 140731178643423 0
 */
static bool stat_proc(w_proc* proc) {
    
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
    free(stat_path);

    struct stat sb;

    if (stat(proc->path, &sb) == -1) {
        fprintf(stderr, "ERROR: could not stat %s: %s\n", proc->path, strerror(errno));
        exit(1);
    }
    proc->uid = sb.st_uid;

    return true;
}

void print_wproc_line(w_proc* proc, t_win list_win) {

    /* "  PID\tUSER PR NI S COMMAND\n"
     *
     * command (1)
     * state   (2)
     * session (6)
     * pr      (17)
     * ni      (18)
     */
    wprintw(
        list_win.win, 
        "  %s\t%s %s %s %s %s\n",
        proc->ptokens[0].value,
        proc->owner_name, 
        proc->ptokens[17].value, 
        proc->ptokens[18].value,
        proc->ptokens[2].value,
        proc->ptokens[1].value
    );
}

// mostra mais informacoes do processo
void print_wproc_win(w_proc* wproc, t_win proc_win) {
    /*  
     * num_threads (19)
     * vsize       (22)
     */
    
    FORMAT(wprintw, proc_win.win, COLOR_PAIR(2) | A_BOLD, "\n  PID %s \n", wproc->ptokens[0].value);
    wprintw(proc_win.win, "  threads: ");
    FORMAT(wprintw, proc_win.win, A_BOLD, "%s\n", wproc->ptokens[19].value);
    wprintw(proc_win.win, "  vsize: ");
    FORMAT(wprintw, proc_win.win, A_BOLD, "%lu", wproc->v_mem);
    wprintw(proc_win.win, " MB, pages: ");
    FORMAT(wprintw, proc_win.win, A_BOLD, "%lu", wproc->pv_mem);
    wprintw(proc_win.win, " total\n");

    wprintw(proc_win.win, "  rss: ");
    FORMAT(wprintw, proc_win.win, A_BOLD, "%lu", wproc->r_mem);
    wprintw(proc_win.win, " MB, pages: ");
    FORMAT(wprintw, proc_win.win, A_BOLD, "%lu", wproc->pr_mem);
    wprintw(proc_win.win, " total\n");

    wprintw(proc_win.win, "  stack: ");
    FORMAT(wprintw, proc_win.win, A_BOLD, "%lu", wproc->stack_size);
    wprintw(proc_win.win, " MB, pages: ");
    FORMAT(wprintw, proc_win.win, A_BOLD, "%lu", wproc->stack_pages);
    wprintw(proc_win.win, " total\n");

    wprintw(proc_win.win, "  heap: ");
    FORMAT(wprintw, proc_win.win, A_BOLD, "%lu", wproc->heap_size);
    wprintw(proc_win.win, " MB, pages: ");
    FORMAT(wprintw, proc_win.win, A_BOLD, "%lu", wproc->heap_pages);
    wprintw(proc_win.win, " total\n");

    wprintw(proc_win.win, "  text/code: ");
    FORMAT(wprintw, proc_win.win, A_BOLD, "%lu", wproc->text_size);
    wprintw(proc_win.win, " MB, pages: ");
    FORMAT(wprintw, proc_win.win, A_BOLD, "%lu", wproc->text_pages);
    wprintw(proc_win.win, " total\n");
}

void proc_free(w_proc* proc) {
    free(proc->path);
    free(proc->owner_name);
}
