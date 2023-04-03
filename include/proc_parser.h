#ifndef __PROC_PARSER_HEADER__
#define __PROC_PARSER_HEADER__
#include <stdio.h>
#define LINESZ 1024

/*
 * estrutura para guardar uma palavra 
 * a ideia é pegar cada token separado 
 * por espacos.
 */
typedef struct token {
    char* value;
} token;

char* trim_l(char* value);
char* trim_r(char* value);
void proc_parse(token *tokens, unsigned long ttokens, FILE* stat_file);

#endif
