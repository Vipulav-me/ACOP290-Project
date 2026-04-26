# ACOP290 - Project : 
# Spreadsheet Program

A terminal-based spreadsheet application written in C that supports formulas,cell dependencies,automatic recalculation,and built-in functions like MIN MAX AVG STD Etc.

## Features

### Core Functionality
- **Grid-based spreadsheet** with up to 999 rows × 18,278 columns (A1 to ZZZ999)
- **Cell values** stored as integers
- **Formulas** with arithmetic operations (+,-,*,/)
- **Cell references** (e.g.,`B1 = A1+5`)
- **Automatic recalculation** when dependencies change for instance if we change the value of a cell which was previously holding some value
- **Circular dependency detection**

### Built-in Functions
| Function | Description | Example |
|----------|-------------|---------|
| `SUM(range)` | Sum of all values in range | `SUM(A1:A10)` |
| `MIN(range)` | Minimum value in range | `MIN(B1:B5)` |
| `MAX(range)` | Maximum value in range | `MAX(C1:C10)` |
| `AVG(range)` | Average (truncated to integer) | `AVG(D1:D20)` |
| `STDEV(range)` | Population standard deviation | `STDEV(E1:E100)` |
| `SLEEP(n)` | Pause execution for n seconds | `SLEEP(2)` |

### Range Support
- **1D column ranges**: `A1:A20`
- **1D row ranges**: `A1:F1`
- **2D rectangular ranges**: `A1:D10`
- Invalid ranges (e.g., `A9:A1`) are rejected

### User Interface
- **Viewport scrolling**: Only 10×10 cells displayed at a time
- **Navigation**: `w`(up), `s`(down), `a`(left), `d`(right)-scrolls by 10
- **Jump to cell**: `scroll_to A1`
- **Output control**: `disable_output`/`enable_output`
- **Timing display**: Shows execution time for each command
- **Error feedback**: Clear error messages for invalid operations

### Error Handling
- Division by zero → displays `ERR`
- **ERR propagation**: Errors cascade through dependent cells
- Circular references → rejected with error message
- Out-of-bounds cells → rejected
- Invalid ranges → rejected
- Parse errors → friendly error messages

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/yourusername/spreadsheet.git
cd spreadsheet

# Build the program
make

# Run with default size (10×10)
./sheet 10 10

# Run with custom size (rows cols)
./sheet 5 5
./sheet 50 100
#And if the user wants to recompile
make clean
