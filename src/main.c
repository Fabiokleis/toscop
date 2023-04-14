#include <stdio.h>
#include "toscop.h"

int main(int argc, char** argv) {
    cli(argc, argv);
    run();
    pthread_exit(NULL);
}
