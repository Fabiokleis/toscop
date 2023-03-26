#ifndef __PROC_PARSER_HEADER__
#define __PROC_PARSER_HEADER__
#include <stdio.h>
#define LINESZ 1024

typedef struct proc_dict {
    int key;
    char* value;
} proc_dict;


void parse_stat(proc_dict pdict[52], FILE* stat_file);
#endif
