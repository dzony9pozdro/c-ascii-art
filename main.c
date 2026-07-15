#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int width;
  int height;
  int maxval;
  int channel_size;
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
int read_pixel(FILE *f, PPMHEADER *header) {

  int rgb[3];
  int brightness;
  uint8_t buf[2]; // P6 .ppm 16 bit channel files are big endian, x86 is little
                  // endian, so we manually assemble the value, high bit first

  for (int i = 0; i < 3; i++) { // 0 - red, 1 - green, 2 - blue
    fread(buf, 1, 2, f);
    rgb[i] = (buf[0] << 8) | buf[1];

    if (rgb[i] > header->maxval) {
      fprintf(
          stderr,
          "channel value incompatible with maxval provided in header: %d > %d",
          rgb[i], header->maxval);
      return 1;
    }
  }

  brightness = (0.299 * rgb[0] + 0.587 * rgb[1] + 0.114 * rgb[2]);

  return 0;
}

int read_ppm_header(FILE *f, PPMHEADER *header) {

  *header = (PPMHEADER){.width = -1,
                        .height = -1,
                        .maxval = -1,
                        .channel_size =
                            -1}; // -1 so unset values fail validate_header

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

  if (header->maxval <= 255) { // ppm files can be 8b or 16b per channel
    header->channel_size = 1;
  } else {
    header->channel_size = 2;
  }

  if (validate_header(header)) { // 0 good 1 bad
    return 1;
  }

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
  read_pixel(f, &header);
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s <file.ppm>\n", argv[0]);
    return 1;
  }

  return read_ppm(argv[1]);
}
