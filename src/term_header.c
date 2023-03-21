#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "term_header.h"
#include "w_task.h"
#include "task_list.h"


// cria e inicializa os campos do th
term_header* create_term_header() {
    term_header *th = malloc(sizeof(term_header));
    th->task_list = create_tasklist();
    sysinfo(&th->si);
    init_tasks(th);

    return th;
}

// inicializa todos as tasks numa lista duplamente encadeada
void init_tasks(term_header* th) {
    int pid, i = 0;

    DIR* proc_d = opendir("/proc");
    struct dirent* pid_f;

    
    if (proc_d == NULL) {
        fprintf(stderr, "ERROR: could not read /proc with opendir: %s\n", strerror(errno));
        exit(1);
    }
   
    // le cada arquivo do /proc que começa com numero, os pids (tasks)
    while ((pid_f = readdir(proc_d))) {
        if (!isdigit(*pid_f->d_name))
            continue;

        pid = strtol(pid_f->d_name, NULL, 10);
       
        th->task_list = add(th->task_list, *create_w_task(pid));
        i++;
    }

    th->t_tasks = i;
    closedir(proc_d);

}

// chama o print da task_list
void tl_print(term_header* th) {
    print_tasklist(th->task_list);
}

// free todos os malloc
void tl_free(term_header* th) {
    free_tasklist(th->task_list); 
    free(th);
}
