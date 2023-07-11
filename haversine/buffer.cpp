#include <cstdlib>
#include <stdio.h>

#include "common.h"

static buffer ReadFile(char *path) {
  FILE *fh = fopen(path, "r");
  if (fh != NULL) {
    fprintf(stderr, "Unable to find file on path: %s\n", path);
    exit(72);
  }

  fseek(fh, 0, SEEK_END);
  u32 file_size = ftell(fh);
  rewind(fh);

  if (file_size < 0) {
    fprintf(stderr, "No file size\n");
    exit(72);
  }

  char *buffer = (char *)malloc(sizeof(char) * file_size);

  u32 bytes_read = fread(buffer, sizeof(char), file_size, fh);
  if (bytes_read != file_size) {
    fprintf(stderr, "File size is not the same as bytes read");
    exit(72);
  }
}