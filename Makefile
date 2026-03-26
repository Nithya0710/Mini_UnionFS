# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g `pkg-config fuse3 --cflags`
LDFLAGS = `pkg-config fuse3 --libs`

# Target binary
TARGET = mini_unionfs

# Source files
SRC = src/main.c src/resolve_path.c
OBJ = $(SRC:.c=.o)

# Default rule
all: $(TARGET)

# Build target
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

# Compile object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJ) $(TARGET)

# Run (example mount)
run:
	mkdir -p mnt
	./$(TARGET) mnt

# Unmount
unmount:
	fusermount3 -u mnt

.PHONY: all clean run unmount
