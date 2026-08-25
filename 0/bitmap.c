#include "bitmap.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// save 24-bits bmp file, buffer must be in bmp format: upside-down
void savebmp(char *name, uchar *buffer, int x, int y) {
  FILE *f = fopen(name, "wb");
  if (!f) {
    printf("Error writing image to disk.\n");
    return;
  }
  unsigned int size = x * y * 3 + 54;
  uchar header[54] = {'B',
                      'M',
                      size & 255,
                      (size >> 8) & 255,
                      (size >> 16) & 255,
                      size >> 24,
                      0,
                      0,
                      0,
                      0,
                      54,
                      0,
                      0,
                      0,
                      40,
                      0,
                      0,
                      0,
                      x & 255,
                      x >> 8,
                      0,
                      0,
                      y & 255,
                      y >> 8,
                      0,
                      0,
                      1,
                      0,
                      24,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0,
                      0};
  fwrite(header, 1, 54, f);
  fwrite(buffer, 1, x * y * 3, f);
  fclose(f);
}

// read bmp file and store image in contiguous array
void readbmp(char *filename, uchar *array) {
  FILE *img = fopen(filename, "rb"); // read the file
  uchar header[54];
  size_t result =
      fread(header, sizeof(uchar), 54, img); // read the 54-byte header

  if (result != 54) {
    perror("Error reading file.\n");
    return;
  }

  // extract image height and width from header
  int width = *(int *)&header[18];
  int height = *(int *)&header[22];
  int padding = 0;
  while ((width * 3 + padding) % 4 != 0)
    padding++;

  int widthnew = width * 3 + padding;
  uchar *data = calloc(widthnew, sizeof(uchar));

  for (int i = 0; i < height; i++) {
    result = fread(data, sizeof(uchar), widthnew, img);

    if (result != (size_t)widthnew) {
      perror("Error reading file.\n");
    }

    for (int j = 0; j < width * 3; j += 3) {
      array[3 * i * width + j + 0] = data[j + 0];
      array[3 * i * width + j + 1] = data[j + 1];
      array[3 * i * width + j + 2] = data[j + 2];
    }
  }
  fclose(img); // close the file
}

void invert(uchar *bmp_arr, size_t size) {
  // iterate throug hall values and mirror the color values (255 - val)
  for (size_t i = 0; i < size; i += 3) {
    bmp_arr[i] = 255 - bmp_arr[i];
  }
}

uchar *shrink(uchar *bmp_arr, size_t size, size_t row_len, size_t factor) {

  // Get the size of array for the new image.
  size_t new_size = size / (factor * factor);

  // Allocate memory for the bmp arr
  uchar *new_bmp = malloc(new_size);

  // the interpolated index of original bmp to read
  size_t interp_val = 0;

  for (size_t i = 0; i < new_size; i += 3) {
    interp_val += (3 * factor);

    // skip the next pixel rows
    if (interp_val % row_len == 0) {
      interp_val += ((factor - 1) * row_len * 3);
    }

    // copy the color vals
    new_bmp[i] = bmp_arr[interp_val];
    new_bmp[i + 1] = bmp_arr[interp_val + 1];
    new_bmp[i + 2] = bmp_arr[interp_val + 2];
  }

  return new_bmp;
}
