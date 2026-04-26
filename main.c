#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include "sheet.h"
#include "parser.h"
#include "formula.h"
#include "deps.h"

// helper to get seconds between two timevals
static double elapsed(struct timeval *start, struct timeval *end) {
    return (end->tv_sec - start->tv_sec) + 
           (end->tv_usec - start->tv_usec) / 1000000.0;
}

int main(int argc, char *argv[]) {
    int rows = 10, cols = 10;
    if (argc == 3) {
        rows = atoi(argv[1]);
        cols = atoi(argv[2]);
        if (rows < 1 || rows > MAX_ROWS) rows = 10;
        if (cols < 1 || cols > MAX_COLS) cols = 10;
    }

    Sheet *sheet = create_sheet(rows, cols);
    print_sheet(sheet);

    char line[1024];
    char status[64] = "ok";
    double last_time = 0.0;

    while (1) {
        // prompt with last command time and status
        printf("[%.1f] (%s) > ", last_time, status);
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }
        line[strcspn(line, "\n")] = 0;

        struct timeval t_start, t_end;
        gettimeofday(&t_start, NULL);
        strcpy(status, "ok");   // assume success

        if (strlen(line) == 0) {
            // empty line, just skip
            gettimeofday(&t_end, NULL);
            last_time = elapsed(&t_start, &t_end);
            continue;
        }

        if (strcmp(line, "q") == 0) {
            break;
        }
        else if (strcmp(line, "w") == 0) {
            if (sheet->view_row >= 10) sheet->view_row -= 10;
            else sheet->view_row = 0;
            if (sheet->output_enabled) print_sheet(sheet);
        }
        else if (strcmp(line, "s") == 0) {
            if (sheet->view_row + 20 <= sheet->rows)
                sheet->view_row += 10;
            else if (sheet->view_row + 10 < sheet->rows)
                sheet->view_row = sheet->rows - 10;
            if (sheet->output_enabled) print_sheet(sheet);
        }
        else if (strcmp(line, "a") == 0) {
            if (sheet->view_col >= 10) sheet->view_col -= 10;
            else sheet->view_col = 0;
            if (sheet->output_enabled) print_sheet(sheet);
        }
        else if (strcmp(line, "d") == 0) {
            if (sheet->view_col + 20 <= sheet->cols)
                sheet->view_col += 10;
            else if (sheet->view_col + 10 < sheet->cols)
                sheet->view_col = sheet->cols - 10;
            if (sheet->output_enabled) print_sheet(sheet);
        }
        else if (strcmp(line, "disable_output") == 0) {
            sheet->output_enabled = 0;
        
        }
        else if (strcmp(line, "enable_output") == 0) {
            sheet->output_enabled = 1;
            ;
        }
        else if (strncmp(line, "scroll_to ", 10) == 0) {
            int r, c;
            if (parse_cell_name(line + 10, &r, &c)) {
                if (r >= 0 && r < sheet->rows && c >= 0 && c < sheet->cols) {
                    sheet->view_row = r;
                    sheet->view_col = c;
                    if (sheet->output_enabled) print_sheet(sheet);
                } else {
                    strcpy(status, "out of bounds");
                }
            } else {
                strcpy(status, "Invalid cell");
            }
        }
        // formula assignment: contains '='
        else if (strchr(line, '=') != NULL) {
            char *eq = strchr(line, '=');
            *eq = '\0';
            char *cell_str = line;
            char *expr = eq + 1;

            int row, col;
            if (parse_cell_name(cell_str, &row, &col)) {
                if (row >= 0 && row < sheet->rows && col >= 0 && col < sheet->cols) {
                    int rc = set_cell_formula(sheet, row, col, expr);
                    switch (rc) {
                        case DEPS_OK: strcpy(status, "ok"); break;
                        case DEPS_ERR_CIRCULAR: strcpy(status, "circular dependency"); break;
                        case DEPS_ERR_DIV0: strcpy(status, "ok"); break;
                        case DEPS_ERR_REF: strcpy(status, "Invalid cell"); break;
                        case DEPS_ERR_RANGE: strcpy(status, "Invalid range"); break;
                        case DEPS_ERR_PARSE: strcpy(status, "unrecognized cmd"); break;
                        default: strcpy(status, "error"); break;
                    }
                    if (sheet->output_enabled) print_sheet(sheet);
                } else {
                    strcpy(status, "out of bounds");
                }
            } else {
                strcpy(status, "invalid cell");
            }
        }
        else if (strlen(line) > 0) {
            strcpy(status, "unrecognized cmd");
        }

        gettimeofday(&t_end, NULL);
        last_time = elapsed(&t_start, &t_end);
    }

    free_sheet(sheet);
    return 0;
}
