#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "deps.h"
#include "formula.h"

//add a reverse dependency - when cell (r,c) is used by "watcher"
static void rdep_add(Sheet *sheet, int r, int c, int watcher) {
    Cell *cell = &sheet->cells[r][c];
    for (int i = 0; i < cell->rdep_count; i++) {
        if (cell->rdeps[i] == watcher) return;   // already there
    }
    // dynamic array for reverse deps, double capacity when full
    if (cell->rdep_count == cell->rdep_cap) {
        int new_cap = (cell->rdep_cap == 0) ? 8 : cell->rdep_cap * 2;
        cell->rdeps = realloc(cell->rdeps, new_cap * sizeof(int));
        cell->rdep_cap = new_cap;
    }
    cell->rdeps[cell->rdep_count++] = watcher;
}

//remooves a reverse dependency
static void rdep_remove(Sheet *sheet, int r, int c, int watcher) {
    Cell *cell = &sheet->cells[r][c];
    for (int i = 0; i < cell->rdep_count; i++) {
        if (cell->rdeps[i] == watcher) {
            // overwrite with last element to keep it compact
            cell->rdeps[i] = cell->rdeps[--cell->rdep_count];
            return;
        }
    }
}

// DFS check: is there a path from 'from' to 'target' in the dependency graph?
static int has_path(Sheet *sheet, int from, int target, char *visited) {
    if (from == target) return 1;
    if (visited[from]) return 0;
    visited[from] = 1;
    int r = decode_row(from), c = decode_col(from);
    Cell *cell = &sheet->cells[r][c];
    for (int i = 0; i < cell->rdep_count; i++) {
        if (has_path(sheet, cell->rdeps[i], target, visited))
            return 1;
    }
    return 0;
}

// Checks if adding the new dependencies would create a cycle
static int creates_cycle(Sheet *sheet, int target, int *deps, int dep_count) {
    int total = sheet->rows * MAX_COLS;
    char *visited = calloc(total, 1);
    if (!visited) return 0;   // can't check, assume no cycle
    for (int i = 0; i < dep_count; i++) {
        memset(visited, 0, total);
        if (has_path(sheet, deps[i], target, visited)) {
            free(visited);
            return 1;
        }
    }
    free(visited);
    return 0;
}

// Recalculate all cells that (transitively) depend on start_enc
static void recalc_downstream(Sheet *sheet, int start_enc) {
    int total = sheet->rows * MAX_COLS;
    int *queue = malloc(total * sizeof(int));
    char *in_queue = calloc(total, 1);
    if (!queue || !in_queue) {
        free(queue);
        free(in_queue);
        return;
    }

    int head = 0, tail = 0;
    int sr = decode_row(start_enc), sc = decode_col(start_enc);
    Cell *start_cell = &sheet->cells[sr][sc];

    // start from direct dependants of the changed cell
    for (int i = 0; i < start_cell->rdep_count; i++) {
        int enc = start_cell->rdeps[i];
        if (!in_queue[enc]) {
            queue[tail++] = enc;
            in_queue[enc] = 1;
        }
    }

    // BFS: evaluate in the order they are discovered – 
    // this works because any dependency of a cell has a shorter path
    // from start_enc, so it's evaluated first....
    while (head < tail) {
        int enc = queue[head++];
        int r = decode_row(enc), c = decode_col(enc);
        Cell *cell = &sheet->cells[r][c];

        if (cell->formula) {
            int deps_buf[MAX_DEPS];
            int dep_count = 0;
            int new_value;
            
            int input_has_error = 0;
            
            int rc = evaluate_formula(cell->formula, sheet, &new_value, deps_buf, &dep_count);
            
            for (int i = 0; i < dep_count; i++) {
                int dr = decode_row(deps_buf[i]), dc = decode_col(deps_buf[i]);
                if (sheet->cells[dr][dc].has_error) {
                    input_has_error = 1;
                    break;
                }
            }
    
            if (input_has_error || rc == EVAL_ERR_DIV0) {
                cell->has_error = 1;
                cell->value = 0;  //manual expects 'ERR' display, which print_sheet handles via has_error
            } else if (rc == EVAL_OK) {
                cell->value = new_value;
                cell->has_error = 0;
            } else if (rc == EVAL_ERR_DIV0) {
                cell->has_error = 1;
                cell->value = 0;
            }
        }

        // add further dependants to the queue
        for (int i = 0; i < cell->rdep_count; i++) {
            int dep = cell->rdeps[i];
            if (!in_queue[dep]) {
                queue[tail++] = dep;
                in_queue[dep] = 1;
            }
        }
    }

    free(queue);
    free(in_queue);
}


int set_cell_formula(Sheet *sheet, int row, int col, const char *expr) {
    int target = encode_cell(row, col);
    Cell *cell = &sheet->cells[row][col];

    //remove old dependencies
    for (int i = 0; i < cell->dep_count; i++) {
        int dep_enc = cell->deps[i];
        rdep_remove(sheet, decode_row(dep_enc), decode_col(dep_enc), target);
    }
    free(cell->deps);
    cell->deps = NULL;
    cell->dep_count = 0;

    // evalss the new formula
    int deps_buf[MAX_DEPS];
    int dep_count = 0;
    int new_value;
    int rc = evaluate_formula(expr, sheet, &new_value, deps_buf, &dep_count);

    if (rc == EVAL_ERR_PARSE) return DEPS_ERR_PARSE;
    if (rc == EVAL_ERR_REF)   return DEPS_ERR_REF;
    if (rc == EVAL_ERR_RANGE) return DEPS_ERR_RANGE;

    //check for circular references
    if (dep_count > 0 && creates_cycle(sheet, target, deps_buf, dep_count))
        return DEPS_ERR_CIRCULAR;

    
    free(cell->formula);
    cell->formula = strdup(expr);
    if (rc == EVAL_OK) {
        cell->value = new_value;
        cell->has_error = 0;
    } else if (rc == EVAL_ERR_DIV0) {
        cell->has_error = 1;
        cell->value = 0;
    }

    //  record dependencies and reverse dependencies
    if (dep_count > 0) {
        cell->deps = malloc(dep_count * sizeof(int));
        memcpy(cell->deps, deps_buf, dep_count * sizeof(int));
        cell->dep_count = dep_count;
        for (int i = 0; i < dep_count; i++) {
            int dep_enc = deps_buf[i];
            rdep_add(sheet, decode_row(dep_enc), decode_col(dep_enc), target);
        }
    }

    //propagate changes to downstream cells
    recalc_downstream(sheet, target);
    return DEPS_OK;
}
