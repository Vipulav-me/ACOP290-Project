#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sheet.h"

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

        // everything else is unknown for now
        printf("unrecognized cmd\n");
    }

    free_sheet(sheet);
    return 0;
}
