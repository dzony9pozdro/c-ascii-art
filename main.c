#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int read_ppm(const char *filename) {
  int status = 0;
  uint8_t *buf = NULL;

  FILE *f = fopen(filename, "rb");
  if (f == NULL) {

    fprintf(stderr, "no such file\n");
    return 1;
  }

  char magic[3];
  if (fscanf(f, "%2s", magic) != 1) {
    fprintf(stderr, "couldn't read magic number\n");
    status = 1;
    goto cleanup;
  }

  if (strcmp(magic, "P6") != 0) {

    fprintf(stderr, "wrong magic number\n");
    status = 1;
    goto cleanup;
  }

  int width, height, maxval;
  if (fscanf(f, "%d %d %d", &width, &height, &maxval) != 3) {
    fprintf(stderr, "reading from file failed\n");
    status = 1;
    goto cleanup;
  }

  if (width <= 0 || height <= 0 || height > 20000 || width > 20000 ||
      maxval <= 0 || maxval >= 65536) {

    fprintf(stderr, "invalid header\n");
    status = 1;
    goto cleanup;
  }

  int channel_size;
  if (maxval <= 255) {
    channel_size = 1;
  } else {
    channel_size = 2;
  }

  size_t size = (size_t)width * height * 3 * channel_size; // *3 for r, g ,b
  buf = malloc(size);
  if (buf == NULL) {
    fprintf(stderr, "malloc failed\n");
    status = 1;
    goto cleanup;
  }

cleanup:
  free(buf);
  fclose(f);
  return status;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s <file.ppm>\n", argv[0]);
    return 1;
  }
  // TODO:first thing to pick up 
  // fgetc, fread - print pixel 0
  return read_ppm(argv[1]);
}
