#include "bitmap.h"
#include <stdio.h>
#include <stdlib.h>

#define XSIZE 2560 // Size of before image
#define YSIZE 2048

int main(void) {
  size_t arr_size = XSIZE * YSIZE * 3;

  uchar *image = calloc(arr_size, 1);
  readbmp("before.bmp", image);

  size_t SHRINK_FACTOR = 8; // must divide 2560 and 2048

  // Alter the image here
  invert(image, arr_size);
  image = shrink(image, arr_size, XSIZE, SHRINK_FACTOR);

  savebmp("after.bmp", image, XSIZE / SHRINK_FACTOR, YSIZE / SHRINK_FACTOR);

  free(image);
  return 0;
}
