CC = gcc
CFLAGS = -Wall -Wextra -g -O2
LDFLAGS = -lm

TARGET = sheet
SOURCES = main.c sheet.c parser.c formula.c deps.c
HEADERS = sheet.h parser.h formula.h deps.h
OBJECTS = $(SOURCES:.c=.o)

TEST_SRC = test_suite.c
TEST_TARGET = test_runner

REPORT_DIR = report
REPORT_SRC = $(REPORT_DIR)/report.tex
REPORT_PDF = $(REPORT_DIR)/report.pdf

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TARGET) $(TEST_TARGET)
	@./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRC) $(TARGET)
	$(CC) $(CFLAGS) -o $(TEST_TARGET) $(TEST_SRC) $(LDFLAGS)

report: $(REPORT_PDF)

report:
	pdflatex report.tex
	pdflatex report.tex

run: $(TARGET)
	@./$(TARGET) 10 10

clean:
	@rm -f $(TARGET) $(OBJECTS) $(TEST_TARGET)
	@rm -f $(REPORT_DIR)/*.aux $(REPORT_DIR)/*.log $(REPORT_DIR)/*.out $(REPORT_DIR)/*.pdf

.PHONY: all test report run clean
