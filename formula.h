#ifndef FORMULA_H
#define FORMULA_H

#include "sheet.h"

// formula.h – parses and evaluates stuff

// some return codes for errors
#define EVAL_OK         0
#define EVAL_ERR_DIV0   1
#define EVAL_ERR_REF    2
#define EVAL_ERR_RANGE  3
#define EVAL_ERR_PARSE  4

// just pass a string and get a result back, also fills deps
int evaluate_formula(const char *expr, Sheet *sheet,
                     int *result, int *dep_buf, int *dep_count);

#endif /* FORMULA_H */
