#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "parser.h"
//This function converts column letters to 0-based indexing
//EG. A:0 AA:26 
int col_name_to_index(const char *col_str){
    if (!col_str||*col_str=='\0'){//Valid input else -1
         return -1;
    }
    int index = 0;
    //Processing each char 
    //Eg: 'AA' is evaluated to ((1*26) + 1) -1 = 26
    //(-1 Due to 0 based index) 
    for (int i=0;col_str[i]!='\0';i++){
        char c = col_str[i];
        if (!isupper(c)){
            return -1; //Only upper case allowed
        }
        index = index*26+(c-'A'+1);
    }
    return index-1; 
}
//This basically does the opposite of the above function 
void index_to_col_name(int index,char *buf){
    char tmp[MAX_CELL_NAME];
    int len = 0;
    index++;
    while(index>0){
        int rem = (index-1)%26;
        tmp[len++] = 'A'+rem;
        index = (index-1)/26;
    }
    for(int i=0; i<len;i++){
        buf[i] = tmp[len-1-i];
    }
    buf[len] ='\0';
}
//What this function does is it parsing the input cell name like Z3 for example
// And converts it to row and column indices 
//For this example: row = 2 , column = 25
int parse_cell_name(const char *cell_str,int *row,int *col) {
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
//Opposite worjing of the above function
//Row column indices to cell name
void cell_name_from_indices(int row, int col, char *buf) {
    char col_str[MAX_CELL_NAME];
    index_to_col_name(col,col_str);
    sprintf(buf,"%s%d",col_str,row+1);
}
