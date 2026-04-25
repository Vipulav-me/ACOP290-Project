#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"

int col_name_to_index(const char *col_str){
    if (!col_str||*col_str=='\0'){
         return -1;
    }
    int index = 0;
    for (int i=0;col_str[i]!='\0';i++){
        char c = col_str[i];
        if (!isupper(c)){
            return -1;
        }
        index = index*26+(c-'A'+1);
    }
    return index-1; 
}

void index_to_col_name(int index, char *buf){
    char tmp[MAX_CELL_NAME];
    int len = 0;
    index++;
    while (index>0){
        int rem = (index-1)%26;
        tmp[len++] = 'A'+rem;
        index = (index-1)/26;
    }
    for (int i=0; i<len;i++){
        buf[i] = tmp[len-1-i];
    }
    buf[len] ='\0';
}

int parse_cell_name(const char *cell_str, int *row, int *col) {
    if (!cell_str||!row||!col){
        return 0;
    }
    char col_buf[MAX_CELL_NAME];
    int col_len = 0;
    int i=0;
    
    while(isupper(cell_str[i])){
        col_buf[col_len++] = cell_str[i++];
        if(col_len >= MAX_CELL_NAME-1){
            return 0;
        } 
    }
    col_buf[col_len]='\0';
    if (col_len == 0) return 0;
    if (!isdigit(cell_str[i])) return 0;
    char *endptr;
    long row_num = strtol(cell_str+i,&endptr,10);    
    if (*endptr !='\0'||row_num<1) return 0;
    if (row_num>MAX_ROWS) return 0;

    int col_index = col_name_to_index(col_buf);
    if (col_index<0||col_index >= MAX_COLS) return 0;
    *col = col_index;
    *row = (int)(row_num-1);
    return 1;
}

void cell_name_from_indices(int row, int col, char *buf) {
    char col_str[MAX_CELL_NAME];
    index_to_col_name(col,col_str);
    sprintf(buf,"%s%d",col_str,row+1);
}