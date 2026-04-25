#ifndef PARSER_H
#define PARSER_H
#define MAX_CELL_NAME 32
#define MAX_ROWS 999
#define MAX_COLS 18278 
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
int col_name_to_index(const char *col_str);
void index_to_col_name(int index,char *buf);
int parse_cell_name(const char *cell_str,int *row,int *col);
void cell_name_from_indices(int row,int col,char *buf);

#endif 