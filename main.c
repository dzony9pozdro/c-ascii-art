#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int width;
  int height;
  int maxval;
} PPMHEADER;

int validate_header(PPMHEADER *header) {
  if (header->width <= 0 || header->width > 20000) {
    fprintf(stderr, "invalid width");
    return 1;
  }
  if (header->height > 20000 || header->height <= 0) {

    fprintf(stderr, "invalid height");
    return 1;
  }
  if (header->maxval <= 0 || header->maxval >= 65536) {

    fprintf(stderr, "invalid maxval");
    return 1;
  }
  return 0;
}
int read_pixel(FILE *f, PPMHEADER *header, int *brightness) {

  int rgb[3];
  uint8_t buf[2]; // P6 .ppm 16 bit channel files are big endian, x86 is little
                  // endian, so we manually assemble the value, high bit first

  for (int i = 0; i < 3; i++) { // 0 - red, 1 - green, 2 - blue

    if (2 != fread(buf, 1, 2, f)) {
      fprintf(stderr, "fread failed\n");
      return 1;
    };

    rgb[i] = (buf[0] << 8) | buf[1];

    if (rgb[i] > header->maxval) {
      fprintf(stderr,
              "channel value incompatible with maxval provided in header: %d > "
              "%d\n",
              rgb[i], header->maxval);
      return 1;
    }
  }

  *brightness = (0.299 * rgb[0] + 0.587 * rgb[1] + 0.114 * rgb[2]);
  // printf("brightness inside read_pixel: %d\n", *brightness);
  return 0;
}

int read_ppm_header(FILE *f, PPMHEADER *header) {

  *header = (PPMHEADER){
      .width = -1,
      .height = -1,
      .maxval = -1,
  }; // -1 so unset values fail validate_header

  char magic[3];
  if (fscanf(f, "%2s", magic) != 1) {
    fprintf(stderr, "couldn't read magic number\n");
    return 1;
  }

  if (strcmp(magic, "P6") != 0) {

    fprintf(stderr, "wrong magic number\n");
    return 1;
  }

  if (fscanf(f, "%d %d %d", &header->width, &header->height, &header->maxval) !=
      3) {
    fprintf(stderr, "failed to retrieve header\n");
    return 1;
  }

  if (validate_header(header)) { // 0 good 1 bad
    return 1;
  }

  return 0;
}

int map_to_palette(int chunk_brightness, char *character, PPMHEADER *header) {

  if (chunk_brightness > header->maxval) {
    fprintf(stderr, "invalid chunk brightness in map_to_palette");
    return 1;
  }

  char *palette = " .:-=+*#%@";
  int palette_length = strlen(palette);

  int idx = chunk_brightness / (header->maxval / 10) - 1;
  if (idx > palette_length - 1) {
    idx = palette_length - 1;
  };
  if (idx < 0) {
    idx = 0;
  }

  *character = palette[idx];
  return 0;
}

int build_row(FILE *f, PPMHEADER *header) {

  uint32_t *acc = calloc(header->width / 10, sizeof(*acc));

  for (int j = 0; j < 20; j++) { // one ascii char per 10x20 pixel block
    for (int i = 0; i < header->width; i++) {
      int b;
      if (0 != read_pixel(f, header, &b)) {
        fprintf(stderr, "failed to read pixel in build_row");
        return 1;
      }
      acc[i / 10] += b;
    }
  }
  int avg;
  char *row = calloc(header->width / 10 + 1, sizeof(*row)); // +1 for \0
  char character;
  for (int i = 0; i < header->width / 10; i++) {
    avg = acc[i] / 200;
    map_to_palette(avg, &character, header);
    row[i] = character;
  }

  printf("%s\n", row);
  free(row);
  free(acc);
  return 0;
}

int read_ppm(const char *filename) {
  int status = 0;
  PPMHEADER header;

  FILE *f = fopen(filename, "rb");

  if (f == NULL) {
    fprintf(stderr, "no such file\n");
    return 1;
  }

  read_ppm_header(f, &header);
  fgetc(f); // get rid of 1 whitespace after maxval
  for (int i = 0; i < header.height / 20; i++) {
    build_row(f, &header);
  }
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s <file.ppm>\n", argv[0]);
    return 1;
  }

  return read_ppm(argv[1]);
}
