#define _GNU_SOURCE
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "../include/proc_parser.h"


// remove espaços a direita de value
static char* trim_r(char *value) {

    int i = 0;
    // quando chegamos num caractere que não é espaço
    // podemos descartar todos os espaços anteriores
    for (i = strlen(value) - 1; isspace(value[i]); i--);
    value[i + 1] = '\0'; // seta para ser o ultimo caractere do char*

    return value;
}

// remove espaços a esquerda de value
static char* trim_l(char *value) {
    while (isspace(*value))
        value++;
    return value;
}

/*
 * faz o parse de uma linha, pegando cada valor 
 * separado por espaco e guardando no token
 * espera que tokens e stat_path sejam validos
 * faz o parse para o token de tipo TYPE_STR 
 */
void proc_parse(token** tokens, uint64_t ttokens, FILE* stat_file) {

    char *line = NULL;
    size_t lsz = 0;
    // apenas uma linha
    if (getline(&line, &lsz, stat_file) == -1) {
        fprintf(stderr, "ERROR: could not getline: %s\n", strerror(errno));
        return; // caso nao consiga ler retorna o tokens vazio sem malloc;
    } 
    int keys = 0; 
    int until_s = 0; 
    int i = 0;
    int pt_c = 0;

    *tokens = (token *) malloc(ttokens * sizeof(token));
    if (tokens == NULL) {
        fprintf(stderr, "ERROR: could not malloc %zu: %s\n", ttokens * sizeof(token), strerror(errno));
        free(line);
        return; // caso nao consiga dar malloc retorna o tokens vazio;
    }

    while (line[i] != '\0' && (uint64_t) keys < ttokens && until_s < MAX_STR_LEN) {

        // controla os parenteses

        if (line[i] == '(')
            pt_c++;

        if (line[i] == ')')
            pt_c--;


        if (isspace(line[i]) && !isspace(line[i - 1]) && pt_c == 0) {
            char aux[until_s];
            (*tokens)[keys].value = malloc(sizeof(char) * (until_s + 1));
            aux[until_s] = '\0';
            for (int k = 0; until_s > 0; until_s--) {
                aux[k] = line[(i - until_s)];
                k++;
            }

            char *clean_str = trim_l(trim_r(aux));
            strncpy((*tokens)[keys].value, clean_str, strlen(clean_str) + 1);
            keys++;
        }

        until_s++; // numero de chars ate um espaco
        i++;
    }
    free(line);
} 

// procura por um char* em um arquivo, caso encontre retorna
// caso nao encontre retorna o token vazio
token find_token(char* name, FILE* qf) {
    assert(name != NULL);
    assert(qf != NULL);

    char *line = NULL;
    size_t lsz = 0;
    ssize_t nread;

    // para procurar no começo do arquivo sempre
    fseek(qf, 0, SEEK_SET);

    while ((nread = getline(&line, &lsz, qf)) != -1) {
        if (strstr(line, name) == line) {

            char* s_val = line + strlen(name); // soma o endereço inicial com o tamanho do name, passando para fim do char*
            s_val = trim_l(s_val); // remove os espaços a esquerda


            // vai até a ocorrencia primeiro espaço antes de terminar o char*
            char* e_val = s_val;
            while (!isspace(*e_val)) {
                e_val++;
            }

            int64_t v_len = e_val - s_val;
            char *value = malloc(v_len + 1);
            memcpy(value, s_val, v_len); // copia o valor para o value
            value[v_len] = '\0';
            free(line);
            return (token){ .value = value };
        }
    }
    free(line);
    return (token){ .value = "" };
}
