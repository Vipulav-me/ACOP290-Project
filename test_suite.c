#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PASS "\033[32mPASS\033[0m"
#define FAIL "\033[31mFAIL\033[0m"

int run_test(const char *name, const char *input, const char *expected){
    char cmd[2048];
    char buffer[1024];
    char output[8192];
    FILE *fp;

    sprintf(cmd,"printf \"%s\\nq\\n\"|./sheet 3 3 2>&1", input);
    fp = popen(cmd,"r");
    if(!fp) return 0;

    output[0] = '\0';

    while(fgets(buffer,sizeof(buffer),fp)){
        strcat(output, buffer);
    }
    pclose(fp);

    int success = strstr(output, expected) != NULL;

    printf("Test: %s\n", name);
    printf("\n");
    printf("Input:\n%s\n\n", input);
    printf("Expected:\n%s\n\n", expected);
    printf("Output:\n%s\n", output);
    printf("\n");
    if(success){
        printf("Result: %s\n", PASS);
        return 1;
    } else {
        printf("Result: %s\n", FAIL);
        return 0;
    }
    printf("\n");
}
int main() {
    int passed = 0, total = 0;
    total++; passed += run_test("Constant assignment", "A1=42", "42");
    total++; passed += run_test("Cell reference", "A1=5\nB1=A1", "5        5");

    total++; passed += run_test("Addition", "A1=3\nB1=A1+2", "5");
    total++; passed += run_test("Subtraction", "A1=10\nB1=A1-3", "7");
    total++; passed += run_test("Multiplication", "A1=4\nB1=A1*5", "20");
    total++; passed += run_test("Integer division", "A1=5/2", "2");

    total++; passed += run_test("Division by zero", "A1=1/0", "ERR");
    total++; passed += run_test("Invalid command", "hello", "unrecognized");
    total++; passed += run_test("Invalid cell", "X9999=5", "invalid");
    total++; passed += run_test("Out of bounds", "A0=5", "invalid");

    total++; passed += run_test("SUM 1D", "A1=1\nA2=2\nA3=3\nB1=SUM(A1:A3)", "6");
    total++; passed += run_test("MAX 1D", "A1=5\nA2=3\nB1=MAX(A1:A2)", "5");
    total++; passed += run_test("MIN 1D", "A1=5\nA2=3\nB1=MIN(A1:A2)", "3");
    total++; passed += run_test("AVG 1D", "A1=1\nA2=2\nA3=6\nB1=AVG(A1:A3)", "3");

    total++; passed += run_test("SUM 2D", 
        "A1=1\nB1=2\nA2=3\nB2=4\nC1=SUM(A1:B2)", "10");

    total++; passed += run_test("AVG 2D", 
        "A1=2\nB1=4\nA2=6\nB2=8\nC1=AVG(A1:B2)", "5");

    total++; passed += run_test("Invalid range reverse", 
        "A1=MAX(A5:A1)", "Invalid range");

    total++; passed += run_test("Recalculation chain", 
        "A1=2\nB1=A1+1\nC1=B1+1\nA1=5", "5        6        7");

    total++; passed += run_test("No unnecessary recalculation", 
        "A1=2\nB1=A1+1\nB1=10\nA1=5", "5       10");

    total++; passed += run_test("Direct circular", "A1=A1+1", "circular");

    total++; passed += run_test("Error propagation", 
        "A1=1\nB1=1/A1\nA1=0", "ERR");

    total++; passed += run_test("Sleep returns value", "A1=SLEEP(2)", "2");
    total++; passed += run_test("Sleep dependency", 
        "A1=2\nB1=SLEEP(A1)", "2");

    total++; passed += run_test("disable/enable output", 
        "disable_output\nA1=5\nB1=A1+5\nenable_output", "10");

    total++; passed += run_test("Scroll right", "d", "B");
    total++; passed += run_test("Scroll down", "s", "2");

    total++; passed += run_test("scroll_to B2", "scroll_to B2", "B");

    total++; passed += run_test("Expression chain (restricted)", 
        "A1=2\nB1=3\nC1=A1+B1", "5");

    total++; passed += run_test("Nested dependency", 
        "A1=2\nB1=A1+3\nC1=B1+A1", "7");

    printf("\n");
    printf("Results: %d/%d tests passed\n", passed, total);
    printf("\n");
    return (passed == total) ? 0 : 1;
}