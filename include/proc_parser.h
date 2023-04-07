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


extern void proc_parse(token *tokens, unsigned long ttokens, FILE* stat_file);
extern token find_token(char* name, FILE* f);

#endif
