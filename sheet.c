#include <stdio.h>
#include <stdlib.h>
#include "sheet.h"
#include "parser.h"  
Sheet* create_sheet(int rows,int cols){
    if (rows<=0 || rows>MAX_ROWS || cols<= 0 || cols>MAX_COLS) {
        return NULL;
    }
    Sheet *sheet = malloc(sizeof(Sheet));
    (*sheet).rows = rows;
    (*sheet).cols = cols;
    (*sheet).view_row = 0;
    (*sheet).view_col = 0;
    (*sheet).output_enabled = 1;
    (*sheet).cells = malloc(rows*sizeof(Cell*));

    for(int i=0;i<rows;i++){
        (*sheet).cells[i] = malloc(cols*sizeof(Cell));

        if (!(*sheet).cells[i]){
            perror("malloc");
            for (int k=0; k<i;k++) {
                free((*sheet).cells[k]);
            }
            free((*sheet).cells);
            free(sheet);
            return NULL;
        }
        for(int j=0;j<cols;j++){
            (*sheet).cells[i][j].value=0;
            (*sheet).cells[i][j].has_error=0;
            (*sheet).cells[i][j].formula=NULL;
            (*sheet).cells[i][j].deps=NULL;
            (*sheet).cells[i][j].dep_count=0;
            (*sheet).cells[i][j].rdeps=NULL;
            (*sheet).cells[i][j].rdep_count=0;
            (*sheet).cells[i][j].rdep_cap=0;
    }}
    return sheet;
}
void print_sheet(const Sheet *sheet)
{
    if (!(*sheet).output_enabled){
        return;
    }
    int r_start = sheet->view_row;
    int c_start = sheet->view_col;
    int r_end = r_start+VIEW_ROWS;
    int c_end = c_start+VIEW_COLS;

    if (r_end>sheet->rows){
        r_end = sheet->rows;
    }
    if (c_end>sheet->cols){
        c_end = sheet->cols;
    }
    printf("%9s", "");
    for(int c=c_start;c<c_end;c++){
        char col_name[MAX_CELL_NAME];
        index_to_col_name(c,col_name);
        printf("%9s",col_name);
    }
    printf("\n");
    for(int r=r_start; r<r_end;r++){
        printf("%9d", r+1);
        for(int c=c_start; c<c_end;c++){
            if (sheet->cells[r][c].has_error) {
                printf("%9s","ERR");
            }else{
                printf("%9d",(*sheet).cells[r][c].value);
            }
        }
        printf("\n");
    }
}
void free_sheet(Sheet *sheet){
    if (!sheet){
        return;
    }
    for (int r=0;r<(*sheet).rows;r++){
        for (int c=0;c<(*sheet).cols;c++){
            free((*sheet).cells[r][c].formula);
            free((*sheet).cells[r][c].deps);
            free((*sheet).cells[r][c].rdeps);
        }
        free((*sheet).cells[r]);
    }
    free((*sheet).cells);
    free(sheet);
}
