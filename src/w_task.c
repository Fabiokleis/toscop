#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <pwd.h>
#include "w_task.h"
#include "term_header.h"

w_task* create_w_task(long int t_pid) { 
    w_task* task = malloc(sizeof(w_task));
    task->pid = t_pid;
    task->path = malloc(sizeof(char) * 12); // /proc + 5
    snprintf(task->path, 12, PROC_PATH"%ld", t_pid);  // copia o path para dentro do buffer

    stat_task(task); // pega informacoes da task uid, gid

    task->prio = getpriority(PRIO_PROCESS, task->pid); // pega prioridade da task

    // pega o nome do usuario e outras informacos com base no uid do stat_task
    struct passwd* r_pwd = malloc(sizeof(struct passwd));
    r_pwd = getpwuid(task->uid);
    if (r_pwd == NULL) {
        fprintf(stderr, "ERROR: could not getpwuid of %ld %s", task->uid, strerror(errno));
    }
    task->owner_name = r_pwd->pw_name; // aponta o onwer_name para o nome no pwd


    return task;
}

void stat_task(w_task* task) {
    struct stat sb;

    if (stat(task->path, &sb) == -1) {
        fprintf(stderr, "ERROR: could not stat %s: %s", task->path, strerror(errno));
        exit(1);
    }
    task->uid = sb.st_uid;
    task->gid = sb.st_gid;
}

void print_wtask(w_task* task) {
    printf("PID %ld UID %ld USER %s PR %ld PATH %s\n", task->pid, task->uid, task->owner_name, task->prio, task->path);
}
