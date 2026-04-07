#include <stdio.h>
#include <stdlib.h>
#include "sheet.h"
Sheet* create_sheet(int rows,int cols){
    Sheet *sheet = malloc(sizeof(Sheet));
    (*sheet).rows = rows;
    (*sheet).cols = cols;
    (*sheet).cells = malloc(rows*sizeof(Cell*));
    for(int i=0;i<rows;i++){
        (*sheet).cells[i] = malloc(cols*sizeof(Cell));
        for(int j=0;j<cols;j++){
            (*sheet).cells[i][j].value=0;
    }}
    return sheet;
}
void print_sheet(const Sheet *sheet)
{
    printf("%9s", "");
    for(int c=0;c<(*sheet).cols;c++)
        printf("%9c",'A'+ c);
    printf("\n");
    for(int r=0; r<(*sheet).rows;r++){
        printf("%9d", r+1);
        for(int c=0; c<(*sheet).cols;c++)
            printf("%9d", (*sheet).cells[r][c].value);
        printf("\n");
    }
}