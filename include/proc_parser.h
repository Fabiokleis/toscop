#ifndef __PROC_PARSER_HEADER__
#define __PROC_PARSER_HEADER__
#include <stdio.h>
#define LINESZ 1024

typedef struct proc_dict {
    int key;
    char* value;
} proc_dict;


void proc_parse(proc_dict pdict[52], FILE* stat_file);
char* trim_v(char* value);
#endif
