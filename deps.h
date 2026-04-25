#ifndef DEPS_H
#define DEPS_H
#include "sheet.h"

// Return codes for the function
#define DEPS_OK 0
#define DEPS_ERR_CIRCULAR 1
#define DEPS_ERR_DIV0 2
#define DEPS_ERR_REF 3
#define DEPS_ERR_RANGE 4
#define DEPS_ERR_PARSE 5

int set_cell_formula(Sheet *sheet, int row, int col, const char *expr);
#endif 