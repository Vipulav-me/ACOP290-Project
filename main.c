#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sheet.h"
#include "parser.h"
#include <ctype.h> 

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
    while (1) {
        printf("> ");               // simple prompt for now
        fflush(stdout);

        if (!fgets(line, sizeof(line), stdin)) {
            printf("\n");
            break;
        }
        line[strcspn(line, "\n")] = 0;

        if (strlen(line) == 0) continue;

        if (strcmp(line, "q") == 0) break;

        else if (strcmp(line, "w") == 0) {
            if (sheet->view_row >= 10) sheet->view_row -= 10;
            else sheet->view_row = 0;
            print_sheet(sheet);
        }
        else if (strcmp(line, "s") == 0) {
            if (sheet->view_row + 20 <= sheet->rows)
                sheet->view_row += 10;
            else if (sheet->view_row + 10 < sheet->rows)
                sheet->view_row = sheet->rows - 10;
            print_sheet(sheet);
        }
        else if (strcmp(line, "a") == 0) {
            if (sheet->view_col >= 10) sheet->view_col -= 10;
            else sheet->view_col = 0;
            print_sheet(sheet);
        }
        else if (strcmp(line, "d") == 0) {
            if (sheet->view_col + 20 <= sheet->cols)
                sheet->view_col += 10;
            else if (sheet->view_col + 10 < sheet->cols)
                sheet->view_col = sheet->cols - 10;
            print_sheet(sheet);
        }
        else if (strcmp(line, "disable_output") == 0) {
            sheet->output_enabled = 0;
            printf("Output disabled\n");   // maybe not needed
        }
        else if (strcmp(line, "enable_output") == 0) {
            sheet->output_enabled = 1;
            print_sheet(sheet);     // show sheet right awayy..
        }
            // jump view to a specific cell
        else if (strncmp(line, "scroll_to ", 10) == 0) {
            int r, c;
            if (parse_cell_name(line + 10, &r, &c)) {
                if (r >= 0 && r < sheet->rows && c >= 0 && c < sheet->cols) {
                    sheet->view_row = r;
                    sheet->view_col = c;
                    if (sheet->output_enabled) print_sheet(sheet);
                } else {
                    printf("out of bounds\n");
                }
            } else {
                printf("Invalid cell\n");
            }
        }

        // everything else is unknown for now
        printf("unrecognized cmd\n");
    }

    free_sheet(sheet);
    return 0;
}
