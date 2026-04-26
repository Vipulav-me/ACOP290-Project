CC = gcc
CFLAGS = -Wall -Wextra -O2 -g
LDFLAGS = -lm

TARGET = sheet
TEST_TARGET = test_runner

SRC = main.c sheet.c parser.c formula.c deps.c
OBJ = $(SRC:.c=.o)

TEST_SRC = test_suite.c

LATEX = pdflatex
REPORT_SRC = report.tex
REPORT_PDF = report.pdf


.PHONY: all clean test run report

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) 10 10

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRC) sheet.c parser.c formula.c deps.c
	$(CC) $(CFLAGS) -o $(TEST_TARGET) $(TEST_SRC) sheet.c parser.c formula.c deps.c $(LDFLAGS)

report: $(REPORT_PDF)

$(REPORT_PDF): $(REPORT_SRC)
	$(LATEX) $(REPORT_SRC)

clean:
	rm -f *.o $(TARGET) $(TEST_TARGET) *.aux *.log *.out $(REPORT_PDF)
