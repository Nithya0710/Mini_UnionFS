CC = gcc
CFLAGS = -Wall -Wextra -g `pkg-config --cflags fuse3`
LIBS = `pkg-config --libs fuse3`

all: mini_unionfs

mini_unionfs: src/main.c src/resolve_path.c
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

test_resolve: tests/test_resolve_path.c
	$(CC) -Wall -g -o test_resolve tests/test_resolve_path.c
	./test_resolve

clean:
	rm -f mini_unionfs test_resolve
