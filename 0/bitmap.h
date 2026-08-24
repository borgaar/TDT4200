#ifndef BITMAP_H
#define BITMAP_H
#include <stddef.h>

typedef unsigned char uchar;

void savebmp(char *name, uchar *buffer, int x, int y);
void readbmp(char *filename, uchar *array);
void invert(uchar *bmp_arr, size_t size);
uchar *shrink(uchar *bmp_arr, size_t size, size_t row_len, size_t factor);

#endif
