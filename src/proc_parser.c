#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <ncurses.h>
#include "proc_parser.h"
#define LINESZ 1024


// faz o parse do /proc/[pid]/stat pegando cada pedaço separados por espacos
/*
 * 32095 (Isolated Web Co) S 1185 1185 1185 0 -1 4194560 78436 0 0 0 2410 1569 0 0 20 0 27 0 923348 2668761088 47885 18446744073709551615 94866003416480 94866003924768 140731178639232 0 0 0 0 69634 1082133752 0 0 0 17 2 0 0 0 0 0 94866003937056 94866003937160 94866029662208 140731178640931 140731178641165 140731178641165 140731178643423 0
 */


// remove espaços a direita de value
char* trim_r(char *value) {
    int i = 0;

    // quando chegamos num caractere que não espaço
    // podemos descatar todos os espaços anteriores
    for (i = strlen(value) - 1; isspace(value[i]); i--);
    value[i + 1] = '\0'; // seta para ser o ultimo caractere do char*

    return value;
}

// remove espaços a esquerda de value
char* trim_l(char *value) {
    while (isspace(*value))
        value++;
    return value;
}


void proc_parse(token ptokens[52], FILE* stat_file) {
    if (ptokens == NULL) {
        fprintf(stderr, "ERROR: proc_dict ptokens is NULL pointer\n");
        exit(1);
    }
    
    if (stat_file == NULL) {
        fprintf(stderr, "ERROR: stat_file is NULL pointer\n");
        exit(1);
    }

    char line[LINESZ]; // o arquivo tem apenas uma linha 
    fgets(line, LINESZ, stat_file);

    int keys = 0; 
    int until_s = 0; 
    int i = 0;
    int pt_c = 0;

    while (line[i] != '\0') {

        if (line[i] == '(')
            pt_c++;

        if (line[i] == ')')
            pt_c--;


        if (isspace(line[i]) && pt_c == 0) {
            ptokens[keys].value = malloc(sizeof(char) * (until_s + 1));
            ptokens[keys].value[until_s] = '\0';
            for (int k = 0; until_s > 0; until_s--) {
                ptokens[keys].value[k] = line[(i - until_s)];
                k++;
            }

            keys++;
        } 

        until_s++;
        i++;
    }

    // da trim em cada value de cada token 
    for (int i = 0; i < 52; i++) {
        ptokens[i].value = trim_l(trim_r(ptokens[i].value));
    }
}
