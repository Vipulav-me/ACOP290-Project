#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "deps.h"
#include "formula.h"

//This function adds a reverse dependency 
//Adds watcher to the rdeps list of cell (r,c). 
 
static void rdep_add(Sheet *sheet,int r,int c,int watcher){
    Cell *cell = &sheet->cells[r][c];
    //First we check if any duplicates are there or no
    // if yes then skip
    for(int i=0;i<cell->rdep_count;i++){
        if(cell->rdeps[i] == watcher) return;
    }
    //We don't knwo how many dependencies are there for one cell
    //so we use a dynamic array
    //if it's full it's size doubles
    if(cell->rdep_count == cell->rdep_cap){
        int new_cap;
        if(cell->rdep_cap == 0){
            new_cap = 8;
        }else{
            new_cap = cell->rdep_cap*2;}
        cell->rdeps = realloc(cell->rdeps,new_cap*sizeof(int));
        cell->rdep_cap = new_cap;
    }
    cell->rdeps[cell->rdep_count++] = watcher;
}

//this function removes a reverse dependency
//Removes watcher from the rdeps list of cell (r,c).

static void rdep_remove(Sheet *sheet,int r,int c,int watcher){
    Cell *cell = &sheet->cells[r][c];
    for(int i=0; i<cell->rdep_count;i++){
        if(cell->rdeps[i] == watcher){
            // Swap with last element and decrement count
            //This way we just consider the n-1 elements now 
            //No need to remove it first and then shift every element
            cell->rdeps[i] = cell->rdeps[--cell->rdep_count];
            return;
        }
    }
}

/*
 This function just uses DFS to check if there's a path from 'from' to 'target'
 Assume the statements 
 A1=1
 B1=A1+7
 C1=MAX(A1:B1)
 Here A1->B1->C1 C1 depends on B1 but does C1 depend on A1 as well
 */
static int has_path(Sheet *sheet,int from,int target,char *visited){
    if(from == target) return 1; //checks if it equal or not
    if(visited[from]) return 0; //visited [from] ? if no then mark it as 1 
    visited[from] = 1;    
    int r = decode_row(from);
    int c = decode_col(from);
    Cell *cell = &sheet->cells[r][c];
    for(int i=0; i<cell->rdep_count;i++){ //iterate over its reverse dependencies
        if(has_path(sheet,cell->rdeps[i],target,visited)){ //if any dependencies has path to target
            //then from also has a path
            return 1;
        }
    }
    return 0;
}

/*
This function checks if adding dependencies would create a cycle
Returns 1 if target appears in any of the dependencies' dependency chains.
 */
static int creates_cycle(Sheet *sheet,int target,int *deps,int dep_count){
    int total = sheet->rows*MAX_COLS;
    char *visited = calloc(total,1);
    if(!visited) return 0; //Assuming no cycle
    for(int i=0;i<dep_count;i++){
        memset(visited,0,total); //Fills visited with total no of 0s
        if(has_path(sheet,deps[i],target,visited)){ //if cycle found then return 1
            // if A1->B1->(can each B) then cycle is found
            free(visited);
            return 1;
        }
    }
    free(visited);
    return 0;
}

/*
This function recalculations after a cell changes
BFS on reverse dependencies,re-evaluating each cell's formula
after all its dependencies are up to date.
 */
static void recalc_downstream(Sheet *sheet,int start_enc){
    int total = sheet->rows*MAX_COLS;
    int *queue = malloc(total*sizeof(int));//It holds the cells to process
    //Again not all cells have to be reevaluated 
    //Only the cells dependent on the changed cell should
    char *in_queue = calloc(total,1);
    if(!queue||!in_queue){
        free(queue);
        free(in_queue);
        return;
    }
    int head=0,tail=0;    
    int sr = decode_row(start_enc);
    int sc = decode_col(start_enc);
    Cell *start_cell = &sheet->cells[sr][sc];
//Getting a dependent cell from rdep
    for (int i=0;i<start_cell->rdep_count;i++){
        int enc = start_cell->rdeps[i];
        if(!in_queue[enc]){
            queue[tail++] = enc;//adding to queue
            in_queue[enc] = 1;//marking in queue
        }
    }
    while(head<tail){
        int enc = queue[head++]; //Next cell to process
        int r = decode_row(enc);
        int c = decode_col(enc);
        Cell *cell = &sheet->cells[r][c];
        //Reevaluating the cell
        if(cell->formula){
            int deps_buf[MAX_DEPS];
            int dep_count = 0;
            int new_value;
            int rc = evaluate_formula(cell->formula,sheet,&new_value,deps_buf,&dep_count);      
            if(rc == EVAL_OK){
                cell->value = new_value;
                cell->has_error = 0;
            }else if(rc == EVAL_ERR_DIV0){
                cell->has_error = 1;
                cell->value = 0;
            }
        }
        //Adding the dependency to queue
        for(int i=0;i<cell->rdep_count;i++){
            int dep = cell->rdeps[i];
            if(!in_queue[dep]){
                queue[tail++] = dep;
                in_queue[dep] = 1;
            }
        }
    }
    free(queue);
    free(in_queue);
}

// Main function
int set_cell_formula(Sheet *sheet,int row,int col,const char *expr){
    int target = encode_cell(row,col);
    Cell *cell = &sheet->cells[row][col];
    
    //Step 1 is to remove old dependency edges
    for(int i=0;i<cell->dep_count;i++){
        int dep_enc = cell->deps[i];
        rdep_remove(sheet,decode_row(dep_enc),decode_col(dep_enc),target);
    }
    free(cell->deps);
    cell->deps = NULL;
    cell->dep_count = 0;
    // Step 2 is to Evaluate new formula 
    int deps_buf[MAX_DEPS];
    int dep_count = 0;
    int new_value;
    int rc = evaluate_formula(expr,sheet,&new_value,deps_buf,&dep_count);
    
    if(rc == EVAL_ERR_PARSE) return DEPS_ERR_PARSE;
    if(rc == EVAL_ERR_REF) return DEPS_ERR_REF;
    if(rc == EVAL_ERR_RANGE) return DEPS_ERR_RANGE;
    
    //Step 3 is to detect any cycles
    if(dep_count>0){
        if(creates_cycle(sheet,target,deps_buf,dep_count)){
            return DEPS_ERR_CIRCULAR;
        }
    }
    
    // Step 4 is to install new formula and value
    free(cell->formula);
    cell->formula = strdup(expr);
    if(rc == EVAL_OK){
        cell->value = new_value;
        cell->has_error = 0;
    } else if(rc == EVAL_ERR_DIV0){
        cell->has_error = 1;
        cell->value = 0;
    }
    
    //Storing dependencies and adding reverse dependencies
    if(dep_count>0){
        cell->deps = malloc(dep_count*sizeof(int));
        memcpy(cell->deps,deps_buf,dep_count*sizeof(int));
        cell->dep_count = dep_count;
        for(int i=0;i<dep_count;i++){
            int dep_enc = deps_buf[i];
            rdep_add(sheet,decode_row(dep_enc),decode_col(dep_enc),target);
        }
    }
    //Step 5 is recalculation
    recalc_downstream(sheet, target);
    return DEPS_OK;
}