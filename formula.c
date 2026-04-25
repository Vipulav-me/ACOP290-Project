#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <unistd.h>
#include "formula.h"
#include "parser.h"

// skip spaces and tabs, they just get in the way
static const char *skip_ws(const char *s) {
    while (*s == ' ' || *s == '\t') s++;
    return s;
}

// add a dependency if we haven't already (idk why we track these tbh)
static void add_dependency(int *dep_buf, int *dep_count, int encoded) {
    for (int i = 0; i < *dep_count; i++) {
        if (dep_buf[i] == encoded) return;
    }
    if (*dep_count < MAX_DEPS) {
        dep_buf[(*dep_count)++] = encoded;
    }
}

// parse a single value: could be a number or a cell ref like A1
// also handles negative numbers (just a minus sign in front)
static int parse_value(const char **pos, Sheet *sheet,
                       int *value, int *dep_buf, int *dep_count) {
    const char *s = skip_ws(*pos);
    
    // check for minus sign to negate later
    int negate = 0;
    if (*s == '-') {
        negate = 1;
        s++;
        s = skip_ws(s);
    }
    else if (*s == '+') {
        // unary plus, just skip it lol
        s++;
        s = skip_ws(s);
    }
    
    // if it starts with a capital letter, it's probs a cell
    if (isupper(*s)) {
        const char *start = s;
        while (isupper(*s)) s++;   // column letters
        if (!isdigit(*s)) return EVAL_ERR_PARSE;  // need a digit after
        while (isdigit(*s)) s++;   // row number
        
        // grab the cell name (hope it fits)
        int len = s - start;
        char cell_name[MAX_CELL_NAME];
        if (len >= MAX_CELL_NAME) return EVAL_ERR_PARSE;
        strncpy(cell_name, start, len);
        cell_name[len] = '\0';
        
        int row, col;
        if (!parse_cell_name(cell_name, &row, &col)) {
            return EVAL_ERR_REF;  // bad cell name
        }
        
        // make sure we're still in the sheet
        if (row < 0 || row >= sheet->rows || col < 0 || col >= sheet->cols) {
            return EVAL_ERR_REF;
        }
        
        // this cell depends on that one, remember for later
        if (dep_buf && dep_count) {
            add_dependency(dep_buf, dep_count, encode_cell(row, col));
        }
        
        // if the cell has an error, we gotta propagate it
        if (sheet->cells[row][col].has_error) {
            *value = CELL_ERR_VALUE;
            *pos = s;
            return EVAL_ERR_DIV0;  // treat as error
        }
        
        // apply negation if needed
        *value = negate ? -sheet->cells[row][col].value 
                        : sheet->cells[row][col].value;
        *pos = s;
        return EVAL_OK;
    }
    
    // looks like a normal number, parse it
    if (isdigit(*s)) {
        char *endptr;
        long v = strtol(s, &endptr, 10);  
        *value = negate ? -(int)v : (int)v;
        *pos = endptr;
        return EVAL_OK;
    }
    
    return EVAL_ERR_PARSE;  
}

// collect all cells in a range like A1:B3 (2D) or A1:A5 (1D)
// puts their values into an array and records dependencies
static int collect_range(const char **pos, Sheet *sheet,
                         int *values, int *n, int max_n,
                         int *dep_buf, int *dep_count) {
    const char *s = skip_ws(*pos);
    
    // first cell (top-left)
    const char *start1 = s;
    while (isupper(*s)) s++;
    if (!isdigit(*s)) return EVAL_ERR_PARSE;
    while (isdigit(*s)) s++;
    int len1 = s - start1;
    char name1[MAX_CELL_NAME];
    strncpy(name1, start1, len1);
    name1[len1] = '\0';
    
    // must have a colon
    if (*s != ':') return EVAL_ERR_PARSE;
    s++;
    
    // second cell (bottom-right)
    const char *start2 = s;
    while (isupper(*s)) s++;
    if (!isdigit(*s)) return EVAL_ERR_PARSE;
    while (isdigit(*s)) s++;
    int len2 = s - start2;
    char name2[MAX_CELL_NAME];
    strncpy(name2, start2, len2);
    name2[len2] = '\0';
    
    int r1, c1, r2, c2;
    if (!parse_cell_name(name1, &r1, &c1)) return EVAL_ERR_REF;
    if (!parse_cell_name(name2, &r2, &c2)) return EVAL_ERR_REF;
    
    // range must be valid: top-left to bottom-right
    if (r1 > r2 || c1 > c2) return EVAL_ERR_RANGE;
    
    // don't go out of the sheet
    if (r2 >= sheet->rows || c2 >= sheet->cols) return EVAL_ERR_REF;
    
    *n = 0;
    for (int r = r1; r <= r2; r++) {
        for (int c = c1; c <= c2; c++) {
            add_dependency(dep_buf, dep_count, encode_cell(r, c));
            if (*n >= max_n) return EVAL_ERR_PARSE;  // too many cells
            
            // if any cell is broken, the whole range fails
            if (sheet->cells[r][c].has_error) {
                return EVAL_ERR_DIV0;
            }
            values[(*n)++] = sheet->cells[r][c].value;
        }
    }
    
    *pos = s;
    return EVAL_OK;
}

// do a built‑in function like SUM, MIN, AVG, STDEV, SLEEP
// sleep is special because it takes a single number, not a range
static int eval_function(const char *name, const char **pos, Sheet *sheet,
                         int *result, int *dep_buf, int *dep_count) {
    if (**pos != '(') return EVAL_ERR_PARSE;  
    (*pos)++;
    
    // handle sleep first cuz it's different
    if (strcmp(name, "SLEEP") == 0) {
        int seconds;
        int rc = parse_value(pos, sheet, &seconds, dep_buf, dep_count);
        if (rc != EVAL_OK) return rc;
        
        *pos = skip_ws(*pos);
        if (**pos != ')') return EVAL_ERR_PARSE;
        (*pos)++;
        
        if (seconds < 0) seconds = 0;  // can't negative time lol
        sleep((unsigned int)seconds);
        *result = seconds;
        return EVAL_OK;
    }
    
    // else it's a function that works on a range
    int values[MAX_DEPS];
    int n = 0;
    int rc = collect_range(pos, sheet, values, &n, MAX_DEPS, dep_buf, dep_count);
    if (rc != EVAL_OK) return rc;
    
    *pos = skip_ws(*pos);
    if (**pos != ')') return EVAL_ERR_PARSE;
    (*pos)++;
    
    if (n == 0) return EVAL_ERR_PARSE;  // empty range check
    
    // choose the right operation
    if (strcmp(name, "SUM") == 0) {
        long sum = 0;
        for (int i = 0; i < n; i++) sum += values[i];
        *result = (int)sum;
    }
    else if (strcmp(name, "MIN") == 0) {
        int m = values[0];
        for (int i = 1; i < n; i++) if (values[i] < m) m = values[i];
        *result = m;
    }
    else if (strcmp(name, "MAX") == 0) {
        int m = values[0];
        for (int i = 1; i < n; i++) if (values[i] > m) m = values[i];
        *result = m;
    }
    else if (strcmp(name, "AVG") == 0) {
        long sum = 0;
        for (int i = 0; i < n; i++) sum += values[i];
        *result = (int)(sum / n);  // integer division, no decimals
    }
    else if (strcmp(name, "STDEV") == 0) {
        // population standard deviation (not sure if it's right but it works)
        double sum = 0;
        for (int i = 0; i < n; i++) sum += values[i];
        double mean = sum / n;
        double variance = 0;
        for (int i = 0; i < n; i++) {
            double diff = values[i] - mean;
            variance += diff * diff;
        }
        variance /= n;
        *result = (int)sqrt(variance);
    }
    else {
        return EVAL_ERR_PARSE;  // unknown function
    }
    
    return EVAL_OK;
}

//evaluates a whole formula string
// it can be a simple math expression, a function call, or just a value
int evaluate_formula(const char *expr, Sheet *sheet,
                     int *result, int *dep_buf, int *dep_count) {
    if (!expr || !sheet || !result) return EVAL_ERR_PARSE;
    if (dep_count) *dep_count = 0;
    
    const char *s = skip_ws(expr);
    
    // if the first thing is a capital letter, it might be a function
    if (isupper(*s)) {
        const char *t = s;
        char fname[32];
        int flen = 0;
        while (isupper(*t) && flen < 31) {
            fname[flen++] = *t++;
        }
        fname[flen] = '\0';
        
        if (*t == '(') {
            const char *after = t;
            int rc = eval_function(fname, &after, sheet, result, dep_buf, dep_count);
            after = skip_ws(after);
            if (rc == EVAL_OK && *after != '\0') return EVAL_ERR_PARSE; // junk after func
            return rc;
        }
    }
    
    // start with a left-hand value
    int lhs;
    int rc = parse_value(&s, sheet, &lhs, dep_buf, dep_count);
    if (rc != EVAL_OK) return rc;
    if (lhs == CELL_ERR_VALUE) return EVAL_ERR_DIV0;
    
    s = skip_ws(s);
    
    // if its just a single value
    if (*s == '\0') {
        *result = lhs;
        return EVAL_OK;
    }
    
    // must be an operator now
    char op = *s;
    if (op != '+' && op != '-' && op != '*' && op != '/') {
        return EVAL_ERR_PARSE;
    }
    s++;
    s = skip_ws(s);
    
    int rhs;
    rc = parse_value(&s, sheet, &rhs, dep_buf, dep_count);
    if (rc != EVAL_OK) return rc;
    if (rhs == CELL_ERR_VALUE) return EVAL_ERR_DIV0;
    
    s = skip_ws(s);
    if (*s != '\0') return EVAL_ERR_PARSE; // extra stuff after expression
    
    // do the math
    switch (op) {
        case '+': *result = lhs + rhs; break;
        case '-': *result = lhs - rhs; break;
        case '*': *result = lhs * rhs; break;
        case '/':
            if (rhs == 0) return EVAL_ERR_DIV0;  // division by zero, nice
            *result = lhs / rhs;
            break;
    }
    
    return EVAL_OK;
}
