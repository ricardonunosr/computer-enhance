#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef unsigned char uchar;

typedef enum { Op_MOV = 0b00100010 } OpCode;

static char *register_names[2][8] = {
    // W = 0
    {
        "al",
        "cl",
        "dl",
        "bl",
        "ah",
        "ch",
        "dh",
        "bh",
    },
    // W = 1
    {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"}};

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Must pass asm file to decode...\n");
    return 0;
  }

  FILE *fd = fopen(argv[1], "r");
  if (fd == NULL) {
    printf("Could not open the asm file...\n");
    return 1;
  }

  fseek(fd, 0, SEEK_END);
  size_t file_size = ftell(fd);
  fseek(fd, 0, SEEK_SET);

  if (file_size == 0) {
    printf("File size is 0...\n");
    return 0;
  }

  printf("File size: %ld \n", file_size);

  char *buffer = (char *)malloc(file_size);
  int result = fread(buffer, 1, file_size, fd);
  if (result < 0 || result != file_size) {
    printf("Could not read file contents into buffer...\n");
    return 1;
  }
  fclose(fd);

  // Dissamble

  // NOTE: use ;unsigned char' because when doing comparison 'char' only goes up
  // to 136 because of sign.
  uint32_t bytes_read = 0;
  while (bytes_read != file_size) {
    uchar byte = *(buffer + bytes_read);
    uchar op_id = byte >> 2;
    switch (op_id) {
    case Op_MOV: {
      // NOTE: 'mov' is actually a copy and not moving.
      uchar d = byte & 0b00000010;
      uchar w = byte & 0b00000001;

      byte = *(buffer + bytes_read + 1);
      bytes_read++;

      uchar mod = (byte & 0b11000000) >> 6;
      uchar reg = (byte & 0b00111000) >> 3;
      uchar r_m = (byte & 0b00000111) >> 0;

      if (mod != 0b00000011) {
        printf("Mod is not 0b11...\n");
        return 1;
      }

      uchar dst = d ? reg : r_m;
      uchar src = d ? r_m : reg;

      char *register_dst = register_names[w][dst];
      char *register_src = register_names[w][src];

      printf("mov %s,%s\n", register_dst, register_src);

    } break;
    default: {
      printf("Unknown OpCode...\n");
      break;
    }
    }

    bytes_read++;
  }
  return 0;
}
