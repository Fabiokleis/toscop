#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <ncurses.h>
#include "../include/proc_parser.h"


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

/*
 * faz o parse de uma linha, pegando cada valor 
 * separado por espaco e guardando no token
 * espera que tokens e stat_path sejam validos
 */
void proc_parse(token* tokens, unsigned long ttokens, FILE* stat_file) {

    char line[LINESZ];
    fgets(line, LINESZ, stat_file);
    int keys = 0; 
    int until_s = 0; 
    int i = 0;
    int pt_c = 0;

    while (line[i] != '\0') {

        // controla os parenteses

        if (line[i] == '(')
            pt_c++;

        if (line[i] == ')')
            pt_c--;


        if (isspace(line[i]) && !isspace(line[i - 1]) && pt_c == 0) {
            tokens[keys].value = malloc(sizeof(char) * (until_s + 1));
            tokens[keys].value[until_s] = '\0';
            for (int k = 0; until_s > 0; until_s--) {
                tokens[keys].value[k] = line[(i - until_s)];
                k++;
            }

            keys++;
        } 

        until_s++; // numero de chars ate um espaco
        i++;
    }

    // da trim em cada value de cada token 
    for (unsigned long i = 0; i < ttokens; i++) {
        tokens[i].value = trim_l(trim_r(tokens[i].value));
    }

} 


