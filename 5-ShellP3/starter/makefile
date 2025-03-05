# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -g

# Target executable names
TARGET = dsh
DRAGON_TARGET = dragon

# Source files for dsh (excluding dragon.c)
DSHSRCS = dsh_cli.c dshlib.c

# Source files for dragon
DRAGON_SRCS = dragon.c

# Header files
HDRS = $(wildcard *.h)

# Default target: build both dsh and dragon
all: $(TARGET) $(DRAGON_TARGET)

# Build dsh shell executable
$(TARGET): $(DSHSRCS) $(HDRS)
	$(CC) $(CFLAGS) -o $(TARGET) $(DSHSRCS)

# Build dragon executable
$(DRAGON_TARGET): $(DRAGON_SRCS) $(HDRS)
	$(CC) $(CFLAGS) -o $(DRAGON_TARGET) $(DRAGON_SRCS)

# Clean up build files
clean:
	rm -f $(TARGET) $(DRAGON_TARGET)

test:
	bats $(wildcard ./bats/*.sh)

valgrind:
	echo "pwd\nexit" | valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./$(TARGET)
	echo "pwd\nexit" | valgrind --tool=helgrind --error-exitcode=1 ./$(TARGET)

.PHONY: all clean test
