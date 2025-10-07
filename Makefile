
# Compiler and flags
CC = gcc
CFLAGS = -Wall

# Folders
SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = obj

# Files
SRC = $(SRC_DIR)/lsv1.0.0.c
OUT = $(BIN_DIR)/ls

# Default rule
all: $(OUT)

$(OUT): $(SRC)
	@echo Compiling...
	$(CC) $(CFLAGS) $(SRC) -o $(OUT)
	@echo Build complete. Run with: .\bin\ls

# Clean up build files
clean:
	@echo Cleaning...
	del /Q $(BIN_DIR)\ls.exe 2>nul || true
	@echo Done.
