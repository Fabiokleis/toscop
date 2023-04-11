#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <ctype.h>
#include "../include/toscop.h"
#include "../include/term_header.h"
#include "../include/toscop_thread.h"
#include "../include/toscop_win.h"

pthread_mutex_t toscop_mutex;
bool fdebug = false;
double max_time = 0; // definida via argv
term_header* th;
term_procs* tp; 
toscop_wm* wm;  

static void print_usage(const char* msg) {
    fprintf(stderr, "%s\nUsage: toscop [-vdN] [-v verboso, -dN tempo de atualização (suporta double)]\n", msg);
    exit(1);
}

// interface com linha de comando para parametrizar algumas coisas
void cli(int argc, char **argv) {
    if (argc > 3) {
        print_usage("Foi passado mais argumentos dos que são possíveis.");
    }

    int arg = 0;
    char *secs = NULL;
    while ((arg = getopt(argc, argv, "vd:")) != -1) { // : indica que precisa de um arg
        switch (arg) {

            case 'v': {
                fdebug = true; // flag para mostrar informacoes de debug na thread de print
            } break;

            case 'd': {
                secs = optarg; // variavel do getopt que guarda o argumento da option
            } break;

            case '?': { // getopt coloca `?` no arg caso alguma coisa deu errado
                if (optopt == 'd') {
                    print_usage("Opção -d precisa do tempo em segundos, mínimo 1.0 sec (-dN).");

                } else if (isprint(optopt)) {
                    print_usage("Opção não existe.");

                } else {
                    print_usage("Unknown option character.");
                }
            } break;

            default:
                print_usage("");
        }
    }

    if (secs != NULL) {
        double t = strtod(secs, NULL);
        max_time = t >= 1 ? t : 3.0; // caso passem um negativo
    } else {
        max_time = 3.0; // default 3 secs
    }
}

// inicializa os structs necessarios, screens, e cria as threads do toscop
// faz o loop principal, o join e limpa todos os recursos utilizados
void run(void) {
    // inicializa ncurses e as window do toscop
    wm = create_toscop_wm(); 

    // inicializa o unico mutex das threads
    pthread_mutex_init(&toscop_mutex, NULL);
    pthread_attr_t attr;     // atributo para dar join

    int s = pthread_attr_init(&attr);

    if (s != 0) {
        fprintf(stderr, "ERROR: could not create attr with pthread_att_init: %s\n", strerror(errno));
        exit(1);
    }

    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    // th tem as informacoes globais
    th = create_term_header();

    // tp tem a lista de processos
    tp = create_term_procs();

    // thread para printar os processos
    toscop_thread_t print_thread;
    print_thread.thread_id = 0;

    int st = pthread_create(&print_thread.thread_id, &attr, print_th, &print_thread); // cria thread com loop principal

    if (st) {
        fprintf(stderr, "ERROR: could not create print thread with pthread_create: %s\n", strerror(errno));
        exit(1);
    }
    // thread para atualizar o th
    toscop_thread_t refresh_thread;
    refresh_thread.thread_id = 1;

    // inicializa a thread que faz o refresh da lista de processos
    st = pthread_create(&refresh_thread.thread_id, &attr, refresh_th, &refresh_thread);
    if (st) {
        fprintf(stderr, "ERROR: could not create refresh thread with pthread_create: %s\n", strerror(errno));
        exit(1);
    }

    // join das threads
    int sj = pthread_join(print_thread.thread_id, NULL);
    if (sj < 0) {
        fprintf(stderr, "ERROR: could not join print_thread: %s\n", strerror(errno));
        exit(1);
    } 

    sj = pthread_join(refresh_thread.thread_id, NULL);
    if (sj < 0) {
        fprintf(stderr, "ERROR: could not join refresh_thread: %s\n", strerror(errno));
        exit(1);
    } 
    // free em tudo que foi usado
    pthread_mutex_destroy(&toscop_mutex);
    pthread_attr_destroy(&attr);
    th_free(th); // limpa infos globais
    tp_free(tp); // limpa lista de procs
    wm_free(wm); // mata as window
    free(wm); // limpa o struct wm
}
