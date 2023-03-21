#include <stdio.h>
#include "term_header.h"


int main() {
    
    term_header* th = create_term_header();    
    printf("************************ TOSCOP *************************\n");
    printf("process count: %d\n", th->si.procs);
    printf("total of tasks (pids): %d\n", th->t_tasks);


    tl_print(th);
    tl_free(th);

    
    return 0;
}
