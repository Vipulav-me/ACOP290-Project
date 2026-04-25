#ifndef SHEET_H
#define SHEET_H

#define MAX_ROWS 999
#define MAX_COLS 18278 
#define VIEW_ROWS 10
#define VIEW_COLS 10
#define MAX_CELL_NAME 32
#define MAX_DEPS 4096
#define CELL_ERR_VALUE (-2147483647-1)

// Cell structure 
/* Each cell has a value 
has_error:  flag that indicates error
formula: Stored formula string
deps: array of cells that this cell depends on
dep_count: no of deps
rdeps: reverse deps
rdep_count: no of rd
rdep_cap: allocated capasity of rdep array
*/
typedef struct {
    int value;
    int has_error;
    char *formula;
    int *deps;
    int dep_count;
    int *rdeps;
    int rdep_count;
    int rdep_cap;
} Cell;

// Sheet structure
// output_enabled: Flag for suppress_output feature
typedef struct {
    int rows;
    int cols;
    Cell **cells;
    int view_row;
    int view_col;
    int output_enabled;
} Sheet;

// Function declarations
Sheet* create_sheet(int rows, int cols);
void print_sheet(const Sheet *sheet);
void free_sheet(Sheet *sheet); 

// Helper functions
static inline int encode_cell(int row, int col) { 
    return row * MAX_COLS + col;
}
static inline int decode_row(int enc) { 
    return enc / MAX_COLS; 
}
static inline int decode_col(int enc) { 
    return enc % MAX_COLS; 
}

#endif