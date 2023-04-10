#include <curses.h>
#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include "../include/w_proc.h"
#include "../include/term_header.h"


static void init_mem(w_proc* proc);
static bool stat_proc(w_proc* proc);
static bool stat_proc_task(w_proc* proc);

w_proc* create_w_proc(uint64_t pid) { 
    w_proc* proc = malloc(sizeof(w_proc));

    uint64_t pid_len = 7 + sizeof(uint64_t); // strlen(PROC_PATH) + pid + \0
    proc->path = malloc(sizeof(char) * pid_len);
    snprintf(proc->path, pid_len, PROC_PATH"%lu", pid);  // copia o path para dentro do buffer
                                                    
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
    
    int p_len = strlen(proc->path) + 6; // proc->path + strlen(/stat) + \0
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

    if (!stat_proc_task(proc)) {
        fprintf(stderr, "ERROR: coult not initiliaze stats of each thread: PID: %s\n", proc->ptokens[0].value);
    }
    return true;
}

// le o /proc/[pid]/task/[tid] e calcula o numero de cada estado das tasks
// retorna false caso nao consiga ler o diretorio
static bool stat_proc_task(w_proc* proc) {
    
    proc->r_threads = 0;
    proc->s_threads = 0;
    proc->z_threads = 0;
    proc->i_threads = 0;
    

    int p_len = strlen(proc->path) + 6; // proc->path + strlen(/task) + \0
    char* task_path = malloc(sizeof(char) * p_len);
    snprintf(task_path, p_len, "%s/task", proc->path);

    DIR* proc_task_dir = opendir(task_path);
    struct dirent* task_id_f;


    // le to /proc/[pid]/task/[tid] e pega o estado de cada task 
    // caso nao consiga ler passa para a proxima task
    while ((task_id_f = readdir(proc_task_dir))) {
        if (!isdigit(*task_id_f->d_name))
            continue;

        uint64_t tid = strtoul(task_id_f->d_name, NULL, 10);
        if (tid == 0) {
            continue;
        }

        uint64_t t_len = strlen(task_path) + sizeof(uint64_t) + 8; // task_path + tid + /status + \0
        char* tid_path = malloc(sizeof(char) * t_len);
        snprintf(tid_path, t_len, "%s/%lu/status", task_path, tid);
        FILE* task_file = fopen(tid_path, "r");


        // caso nao consiga ler essa task vai para proxima se tiver
        if (task_file == NULL) {
            fprintf(stderr, "%lu ERROR: could not read %s with fopen: %s\n", tid, tid_path, strerror(errno));
            free(tid_path); // limpa o que fez o snprintf path
            continue;
        }

        token state = find_token("State:", task_file);

        switch (state.value[0]) {
            case 'S':
                proc->s_threads++;
                break;
            case 'Z':
                proc->z_threads++;
                break;
            case 'I':
                proc->i_threads++;
                break;
            case 'R':
                proc->r_threads++;
                break;
            default:
                break;
        }

        free(tid_path);
        fclose(task_file);
    }
    free(task_path);

    if (proc_task_dir == NULL) {
        fprintf(stderr, "ERROR: could not read /proc/%s/task with opendir: %s\n", proc->ptokens[0].value, strerror(errno));
        return false;
    }

    closedir(proc_task_dir);
    return true;
}

void print_wproc_line(w_proc* proc, t_win list_win) {

    /* "     PID USER      PR NI S COMMAND\n"
     *
     * command (1)
     * state   (2)
     * session (6)
     * pr      (17)
     * ni      (18)
     */
    wprintw(
        list_win.win, 
        " %7s %-8.8s %4s %3s %1s %s\n",
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
    FORMAT(wprintw, proc_win.win, A_BOLD, "%s", wproc->ptokens[19].value);
    wprintw(proc_win.win, " total, ");
    FORMAT(wprintw, proc_win.win, A_BOLD, "%lu", wproc->r_threads);
    wprintw(proc_win.win, " R, ");
    FORMAT(wprintw, proc_win.win, A_BOLD, "%lu", wproc->s_threads);
    wprintw(proc_win.win, " S, ");
    FORMAT(wprintw, proc_win.win, A_BOLD, "%lu", wproc->z_threads);
    wprintw(proc_win.win, " Z, ");
    FORMAT(wprintw, proc_win.win, A_BOLD, "%lu", wproc->i_threads);
    wprintw(proc_win.win, " I\n");
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
    wprintw(proc_win.win, " kB, pages: ");
    FORMAT(wprintw, proc_win.win, A_BOLD, "%lu", wproc->stack_pages);
    wprintw(proc_win.win, " total\n");

    wprintw(proc_win.win, "  heap: ");
    FORMAT(wprintw, proc_win.win, A_BOLD, "%lu", wproc->heap_size);
    wprintw(proc_win.win, " kB, pages: ");
    FORMAT(wprintw, proc_win.win, A_BOLD, "%lu", wproc->heap_pages);
    wprintw(proc_win.win, " total\n");

    wprintw(proc_win.win, "  text/code: ");
    FORMAT(wprintw, proc_win.win, A_BOLD, "%lu", wproc->text_size);
    wprintw(proc_win.win, " kB, pages: ");
    FORMAT(wprintw, proc_win.win, A_BOLD, "%lu", wproc->text_pages);
    wprintw(proc_win.win, " total\n");
}

void proc_free(w_proc* proc) {
    free(proc->path);
    free(proc->owner_name);
}
