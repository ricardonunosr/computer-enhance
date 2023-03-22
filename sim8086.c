#include "common.h"
#include "stdlib.h"
#include <stdio.h>

typedef enum Opcode
{
  Op_INVALID,
  Op_MOV_RegMemToFromReg,
  Op_MOV_ImmToRegMem,
  Op_MOV_ImmToReg,
  Op_MOV_MemToAccum,
  Op_MOV_AccumToMem,
} Opcode;

static bool opcode_lookup_initialized;
static u8 opcode_lookup[256];

// Example
// Op_MOV_RegMemToFromReg can take the values:
// 0b10001000
// 0b10001001
// 0b10001010
// 0b10001011
// and we add them to the opcode_lookup.
void RegisterOpcode(u8 pattern, u8 pattern_length, Opcode opcode)
{
  u8 spare_bits = 8 - pattern_length;
  u32 fill_count = 1 << spare_bits;

  for (u64 fill_pattern = 0; fill_pattern < fill_count; fill_pattern++)
  {
    u8 lookup_pattern = (u8)((pattern << spare_bits) | fill_pattern);
    opcode_lookup[lookup_pattern] = opcode;
  }
}

void InitializeOpcodeLookup(void)
{
  RegisterOpcode(0x22 /* 0b100010dw */, 6, Op_MOV_RegMemToFromReg);
  RegisterOpcode(0x63 /* 0b1100011w */, 7, Op_MOV_ImmToRegMem);
  RegisterOpcode(0x0B /* 0b1011wreg */, 4, Op_MOV_ImmToReg);
  RegisterOpcode(0x50 /* 0b1010000w */, 7, Op_MOV_MemToAccum);
  RegisterOpcode(0x51 /* 0b1010001w */, 7, Op_MOV_AccumToMem);

  opcode_lookup_initialized = true;
}

static s8 *register_names[2][8] = {
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

static s8 *effective_memory_names[8] = {
    "bx + si", "bx + di", "bp + si", "bp + di", "si", "di", "bp", "bx"};

typedef struct Dissambler
{
  s8 *buffer;
  u32 bytes_read;
} Dissambler;

u8 ReadU8(Dissambler *state)
{
  u8 byte = *(state->buffer + state->bytes_read);
  state->bytes_read++;
  return byte;
}

u16 ReadU16(Dissambler *state)
{
  u16 result = ReadU8(state) | (ReadU8(state) << 8);
  return result;
}

s8 ReadS8(Dissambler *state)
{
  return (s8)ReadU8(state);
}

s16 ReadS16(Dissambler *state)
{
  return (s16)ReadU16(state);
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    printf("Must pass asm file to decode...\n");
    return 0;
  }

  FILE *fd = fopen(argv[1], "r");
  if (fd == NULL)
  {
    printf("Could not open the asm file...\n");
    return 1;
  }

  fseek(fd, 0, SEEK_END);
  size_t file_size = ftell(fd);
  fseek(fd, 0, SEEK_SET);

  if (file_size == 0)
  {
    printf("File size is 0...\n");
    return 0;
  }

  s8 *buffer = (s8 *)malloc(file_size);
  int result = fread(buffer, 1, file_size, fd);
  if (result < 0 || result != file_size)
  {
    printf("Could not read file contents into buffer...\n");
    return 1;
  }
  fclose(fd);

  // Dissamble
  //
  if (!opcode_lookup_initialized)
  {
    InitializeOpcodeLookup();
  }

  Dissambler *state = &(Dissambler){.buffer = buffer, .bytes_read = 0};

  // NOTE: use 'unsigned char' because when doing comparison 'char' only goes up
  // to 136 because of sign.
  while (state->bytes_read != file_size)
  {
    u8 byte = ReadU8(state);

    Opcode op = opcode_lookup[byte];

    switch (op)
    {
    case Op_MOV_RegMemToFromReg:
    {
      // NOTE: 'mov' is actually a copy and not moving.
      // MOV destination,source
      u8 d = byte & 0b00000010;
      u8 w = byte & 0b00000001;

      u8 byte = ReadU8(state);

      u8 mod = (byte & 0b11000000) >> 6;
      u8 reg = (byte & 0b00111000) >> 3;
      u8 r_m = (byte & 0b00000111) >> 0;

      if (mod == 0b00000011)
      {
        // Register to Register
        u8 dst = d ? reg : r_m;
        u8 src = d ? r_m : reg;

        s8 *register_dst = register_names[w][dst];
        s8 *register_src = register_names[w][src];

        printf("mov %s,%s\n", register_dst, register_src);
      }
      else
      {
        // Effective address
        s8 *register_name = register_names[w][reg];

        printf("mov ");
        if (mod == 0 && r_m == 0b00000110)
        {
          // Direct Address
          s16 disp = ReadU16(state);
          printf("%s, [%d]", register_name, disp);
        }
        else
        {
          s16 disp = 0;
          if (mod != 0)
            disp = mod == 1 ? ReadS8(state) : ReadS16(state);

          if (d)
          {
            printf("%s, ", register_name);
            if (disp)
            {
              printf("[%s %c %d]", effective_memory_names[r_m], disp >= 0 ? '+' : '-', disp >= 0 ? disp : -disp);
            }
            else
            {
              printf("[%s]", effective_memory_names[r_m]);
            }
          }
          else
          {
            if (disp)
            {
              printf("[%s %c %d]", effective_memory_names[r_m], disp >= 0 ? '+' : '-', disp >= 0 ? disp : -disp);
            }
            else
            {
              printf("[%s]", effective_memory_names[r_m]);
            }
            printf(", %s", register_name);
          }
        }
        printf("\n");
      }
    }
    break;

    case Op_MOV_ImmToRegMem:
    {
      u8 w = (byte & 0b00000001) >> 0;

      u8 byte = ReadU8(state);

      u8 mod = (byte & 0b11000000) >> 6;
      u8 r_m = (byte & 0b00000111) >> 0;

      s16 disp = 0;
      if (mod != 0)
        disp = mod == 1 ? ReadS8(state) : ReadS16(state);

      s16 data = w ? ReadU16(state) : ReadU8(state);

      printf("mov ");

      if (mod == 0b00000011)
      {
        s8 *register_dst = register_names[w][r_m];
        printf("%s, ", register_dst);
      }
      else
      {
        printf("[%s", effective_memory_names[r_m]);
        if (disp)
        {
          printf(" + %d], ", disp);
        }
        else
        {
          printf("], ");
        }
      }

      if (w)
      {
        printf("word %d\n", data);
      }
      else
      {
        printf("byte %d\n", data);
      }
    }
    break;

    case Op_MOV_ImmToReg:
    {
      u8 w = (byte & 0b00001000) >> 3;
      u8 reg = (byte & 0b00000111) >> 0;

      s8 *register_dst = register_names[w][reg];

      s16 data = w ? ReadU16(state) : ReadU8(state);

      printf("mov %s,%d\n", register_dst, data);
    }
    break;

    case Op_MOV_MemToAccum:
    {
      u8 w = (byte & 0b00000001) >> 0;

      u16 addr = ReadU16(state);

      printf("mov %s, [%d]\n", w ? "ax" : "al", addr);
    }
    break;

    case Op_MOV_AccumToMem:
    {
      u8 w = (byte & 0b00000001) >> 0;

      u16 addr = ReadU16(state);

      printf("mov [%d], %s\n", addr, w ? "ax" : "al");
    }
    break;

    default:
    {
      printf("Unknown OpCode...\n");
    }
    break;
    }
  }
  return 0;
}
