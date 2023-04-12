#ifndef __PROC_PARSER_HEADER__
#define __PROC_PARSER_HEADER__
#include <stdio.h>
#include <stdint.h>
#define MAX_STR_LEN 128
/*
 * estrutura para guardar uma palavra 
 * a ideia é pegar cada token separado 
 * por espacos.
 */
typedef struct token {
    char* value;
} token;


extern void proc_parse(token** tokens, uint64_t ttokens, FILE* stat_file);
extern token find_token(char* name, FILE* qf);

#endif
