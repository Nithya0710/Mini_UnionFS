CC = gcc
CFLAGS = -Wall -Wextra -g `pkg-config --cflags fuse3`
LIBS = `pkg-config --libs fuse3`

all: mini_unionfs

mini_unionfs: src/main.c src/resolve_path.c
	$(CC) $(CFLAGS) -o mini_unionfs $^ $(LIBS)

clean:
	rm -f mini_unionfs
