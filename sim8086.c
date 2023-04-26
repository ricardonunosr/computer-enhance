#include "common.h"
#include "stdlib.h"
#include <stdio.h>

typedef enum Opcode {
  Op_INVALID,
  // mov
  Op_MOV_RegMemToFromReg,
  Op_MOV_ImmToRegMem,
  Op_MOV_ImmToReg,
  Op_MOV_MemToAccum,
  Op_MOV_AccumToMem,
  // add
  Op_ADD_RegMemWithReg,
  Op_ADD_ImmToRegMem,
  Op_ADD_ImmToAccum,
  // sub
  Op_SUB_RegMemWithReg,
  Op_SUB_ImmToRegMem,
  Op_SUB_ImmToAccum,
  // cmp
  Op_CMP_RegMemWithReg,
  Op_CMP_ImmToRegMem,
  Op_CMP_ImmToAccum,
  // jmp
  Op_JE_JZ,
  Op_JL_JNZ,
  Op_JLE_JNG,
  Op_JB_JNAE,
  Op_JBE_JNA,
  Op_JP_JPE,
  Op_JO,
  Op_JS,
  Op_JNE_JNZ,
  Op_JNL_JGE,
  Op_JNLE_JG,
  Op_JNB_JAE,
  Op_JNBE_JA,
  Op_JNP_JPO,
  Op_JNO,
  Op_JNS,
  Op_LOOP,
  Op_LOOPZ_LOOPE,
  Op_LOOPNZ_LOOPNE,
  Op_JCXZ
} Opcode;

typedef struct Pattern {
  u8 pattern;
  u8 pattern_length;
  s8 *mnemonic;
  Opcode opcode;
  u8 decode_flags;
  bool peak_next_byte;
  u8 pattern_next_byte;
} Pattern;

Pattern patterns[] = {
    // clang-format off
    {.pattern = 0b10001000 /* 0b100010dw */,.pattern_length = 6, .mnemonic = "mov", .opcode = Op_MOV_RegMemToFromReg},
    {.pattern = 0b11000110 /* 0b1100011w */,.pattern_length = 7, .mnemonic = "mov", .opcode = Op_MOV_ImmToRegMem},
    {.pattern = 0b10110000 /* 0b1011wreg */,.pattern_length = 4, .mnemonic = "mov", .opcode = Op_MOV_ImmToReg},
    {.pattern = 0b10100000 /* 0b1010000w */,.pattern_length = 7, .mnemonic = "mov", .opcode = Op_MOV_MemToAccum},
    {.pattern = 0b10100010 /* 0b1010001w */,.pattern_length = 7, .mnemonic = "mov", .opcode = Op_MOV_AccumToMem},

    {.pattern = 0b00000000 /* 0b000000dw */,.pattern_length = 6, .mnemonic = "add", .opcode = Op_ADD_RegMemWithReg},
    {.pattern = 0b10000000 /* 0b100000sw */,.pattern_length = 6, .mnemonic = "add", .opcode = Op_ADD_ImmToRegMem, .decode_flags = 1, .peak_next_byte = true, .pattern_next_byte = 0b00000000},
    {.pattern = 0b00000100 /* 0b0000010w */,.pattern_length = 7, .mnemonic = "add", .opcode = Op_ADD_ImmToAccum},

    {.pattern = 0b00101000 /* 0b001010dw */,.pattern_length = 6, .mnemonic = "sub", .opcode = Op_SUB_RegMemWithReg},
    {.pattern = 0b10000000 /* 0b100000sw */,.pattern_length = 6, .mnemonic = "sub", .opcode = Op_SUB_ImmToRegMem, .decode_flags = 1, .peak_next_byte = true, .pattern_next_byte = 0b00101000},
    {.pattern = 0b00101100 /* 0b0010110w */,.pattern_length = 7, .mnemonic = "sub", .opcode = Op_SUB_ImmToAccum},

    {.pattern = 0b00111000 /* 0b001110dw */,.pattern_length = 6, .mnemonic = "cmp", .opcode = Op_CMP_RegMemWithReg},
    {.pattern = 0b10000000 /* 0b100000sw */,.pattern_length = 6, .mnemonic = "cmp", .opcode = Op_CMP_ImmToRegMem, .decode_flags = 1, .peak_next_byte = true, .pattern_next_byte = 0b00111000},
    {.pattern = 0b00111100 /* 0b0011110w */,.pattern_length = 7, .mnemonic = "cmp", .opcode = Op_CMP_ImmToAccum},

    {.pattern = 0b01110100 /* 0b01110100 */,.pattern_length = 8, .mnemonic = "je" /* JE/JZ */, .opcode = Op_JE_JZ},
    {.pattern = 0b01111100 /* 0b01111100 */,.pattern_length = 8, .mnemonic = "jl" /* JL/JNGE */, .opcode = Op_JL_JNZ},
    {.pattern = 0b01111110 /* 0b01111110 */,.pattern_length = 8, .mnemonic = "jle"/* JLE/JNG */, .opcode = Op_JLE_JNG},
    {.pattern = 0b01110010 /* 0b01110010 */,.pattern_length = 8, .mnemonic = "jb"/* JB/JNAE */, .opcode = Op_JB_JNAE},
    {.pattern = 0b01110110 /* 0b01110110 */,.pattern_length = 8, .mnemonic = "jbe"/* JBE/JNA */, .opcode = Op_JBE_JNA},
    {.pattern = 0b01111010 /* 0b01111010 */,.pattern_length = 8, .mnemonic = "jp"/* JP/JPE */, .opcode = Op_JP_JPE},
    {.pattern = 0b01110000 /* 0b01110000 */,.pattern_length = 8, .mnemonic = "jo"/* JO */, .opcode = Op_JO},
    {.pattern = 0b01111000 /* 0b01111000 */,.pattern_length = 8, .mnemonic = "js"/* JS */, .opcode = Op_JS},
    {.pattern = 0b01110101 /* 0b01110101 */,.pattern_length = 8, .mnemonic = "jne"/* JNE/JNZ */, .opcode = Op_JNE_JNZ},
    {.pattern = 0b01111101 /* 0b01111101 */,.pattern_length = 8, .mnemonic = "jnl"/* JNL/JGE */, .opcode = Op_JNL_JGE},
    {.pattern = 0b01111111 /* 0b01111111 */,.pattern_length = 8, .mnemonic = "jg"/* JNLE/JG */, .opcode = Op_JNLE_JG},
    {.pattern = 0b01110011 /* 0b01110011 */,.pattern_length = 8, .mnemonic = "jnb"/* JNB/JAE */, .opcode = Op_JNB_JAE},
    {.pattern = 0b01110111 /* 0b01110111 */,.pattern_length = 8, .mnemonic = "ja"/* JNBE/JA */, .opcode = Op_JNBE_JA},
    {.pattern = 0b01111011 /* 0b01111011 */,.pattern_length = 8, .mnemonic = "jnp"/* JNP/JPO */, .opcode = Op_JNP_JPO},
    {.pattern = 0b01110001 /* 0b01110001 */,.pattern_length = 8, .mnemonic = "jno"/* JNO */, .opcode = Op_JNO},
    {.pattern = 0b01111001 /* 0b01111001 */,.pattern_length = 8, .mnemonic = "jns"/* JNS */, .opcode = Op_JNS},
    {.pattern = 0b11100010 /* 0b11100010 */,.pattern_length = 8, .mnemonic = "loop"/* LOOP */, .opcode = Op_LOOP},
    {.pattern = 0b11100001 /* 0b11100001 */,.pattern_length = 8, .mnemonic = "loopz"/* LOOPZ/LOOPE */, .opcode = Op_LOOPZ_LOOPE},
    {.pattern = 0b11100000 /* 0b11100000 */,.pattern_length = 8, .mnemonic = "loopnz"/* LOOPNZ/LOOPNE */, .opcode = Op_LOOPNZ_LOOPNE},
    {.pattern = 0b11100011 /* 0b11100011 */,.pattern_length = 8, .mnemonic = "jcxz"/* JCXZ */, .opcode = Op_JCXZ},
    // clang-format on
};

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

typedef struct Dissambler {
  s8 *buffer;
  u32 bytes_read;
} Dissambler;

u8 ReadU8(Dissambler *state) {
  u8 byte = *(state->buffer + state->bytes_read);
  state->bytes_read++;
  return byte;
}

u16 ReadU16(Dissambler *state) {
  u16 result = ReadU8(state) | (ReadU8(state) << 8);
  return result;
}

s8 ReadS8(Dissambler *state) { return (s8)ReadU8(state); }

s16 ReadS16(Dissambler *state) { return (s16)ReadU16(state); }

u8 PeakU8(Dissambler *state) {
  u8 result = ReadU8(state);
  state->bytes_read--;
  return result;
}

Pattern *TryDecode(u8 byte, Dissambler *state) {
  for (int i = 0; i < ArrayCount(patterns); ++i) {
    Pattern *pattern = &patterns[i];
    u8 shift_byte = byte >> (8 - pattern->pattern_length);
    u8 shift_pattern = pattern->pattern >> (8 - pattern->pattern_length);
    if (shift_pattern == shift_byte) {
      if (pattern->peak_next_byte) {
        u8 next_byte = PeakU8(state);
        u8 byte_pattern = (next_byte & 0b00111000) >> 3;
        u8 pattern_next_byte = (pattern->pattern_next_byte & 0b00111000) >> 3;
        if (byte_pattern == pattern_next_byte)
          return pattern;
      } else {
        return pattern;
      }
    }
  }
  return NULL;
}

#define NUM_REGISTERS 8

typedef struct CPU {
  u16 registers[NUM_REGISTERS];
}CPU;

void PrintCPUState(CPU* cpu) {
  printf("\nFinal Registers:\n");
  for( int i = 0; i < NUM_REGISTERS; ++i) {
    printf("\t%s 0x%08x (%d)\n", register_names[1][i], cpu->registers[i], cpu->registers[i]);
  }
}

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

  s8 *buffer = (s8 *)malloc(file_size);
  int result = fread(buffer, 1, file_size, fd);
  if (result < 0 || result != file_size) {
    printf("Could not read file contents into buffer...\n");
    return 1;
  }
  fclose(fd);

  // Dissamble
  //
  Dissambler *state = &(Dissambler){.buffer = buffer, .bytes_read = 0};

  // CPU
  //
  CPU *cpu = &(CPU){};

  // NOTE: use 'unsigned char' because when doing comparison 'char' only goes up
  // to 136 because of sign.
  while (state->bytes_read != file_size) {
    u8 byte = ReadU8(state);

    Pattern *pattern_found = TryDecode(byte, state);

    if (pattern_found == NULL) {
      printf("Pattern not found ...\n");
      u8 next_byte = ReadU8(state);
      printf("Byte: 0x%08x\n", byte);
      printf("Next byte: 0x%08x\n", next_byte);
    }

    // NOTE: 'mov' is actually a copy and not moving.
    // MOV destination, source
    switch (pattern_found->opcode) {
    case Op_MOV_RegMemToFromReg:
    case Op_SUB_RegMemWithReg:
    case Op_CMP_RegMemWithReg:
    case Op_ADD_RegMemWithReg: {
      u8 d = byte & 0b00000010;
      u8 w = byte & 0b00000001;

      u8 byte = ReadU8(state);

      u8 mod = (byte & 0b11000000) >> 6;
      u8 reg = (byte & 0b00111000) >> 3;
      u8 r_m = (byte & 0b00000111) >> 0;

      if (mod == 0b00000011) {
        // Register to Register
        u8 dst = d ? reg : r_m;
        u8 src = d ? r_m : reg;

        s8 *reg_dst = register_names[w][dst];
        s8 *reg_src = register_names[w][src];

        printf("%s %s, %s\n", pattern_found->mnemonic, reg_dst,
               reg_src);
        cpu->registers[dst] = cpu->registers[src];
      } else {
        // Effective address
        s8 *register_name = register_names[w][reg];

        printf("%s ", pattern_found->mnemonic);
        if (mod == 0 && r_m == 0b00000110) {
          // Direct Address
          s16 disp = ReadU16(state);
          printf("%s, [%d]", register_name, disp);
        } else {
          s16 disp = 0;
          if (mod != 0)
            disp = mod == 1 ? ReadS8(state) : ReadS16(state);

          if (d) {
            printf("%s, ", register_name);
            if (disp) {
              printf("[%s %c %d]", effective_memory_names[r_m],
                     disp >= 0 ? '+' : '-', disp >= 0 ? disp : -disp);
            } else {
              printf("[%s]", effective_memory_names[r_m]);
            }
          } else {
            if (disp) {
              printf("[%s %c %d]", effective_memory_names[r_m],
                     disp >= 0 ? '+' : '-', disp >= 0 ? disp : -disp);
            } else {
              printf("[%s]", effective_memory_names[r_m]);
            }
            printf(", %s", register_name);
          }
        }
        printf("\n");
      }
    } break;

    case Op_MOV_ImmToRegMem:
    case Op_SUB_ImmToRegMem:
    case Op_CMP_ImmToRegMem:
    case Op_ADD_ImmToRegMem: {
      s16 s = 0;
      if (pattern_found->decode_flags) {
        s = (byte >> 1) & 0x1;
      }

      u8 w = (byte & 0b00000001) >> 0;

      u8 byte = ReadU8(state);

      u8 mod = (byte & 0b11000000) >> 6;
      u8 r_m = (byte & 0b00000111) >> 0;

      s16 disp = 0;
      if (mod != 0 && mod != 0x3)
        disp = mod == 1 ? ReadS8(state) : ReadS16(state);

      printf("%s ", pattern_found->mnemonic);

      if (mod == 0b00000011) {
        s8 *register_dst = register_names[w][r_m];
        printf("%s, ", register_dst);
      } else {
        w ? printf("word ") : printf("byte ");
        if (mod == 0 && r_m == 0b00000110) {
          // Direct Address
          s16 disp = ReadU16(state);
          printf("[%d", disp);
        } else {
        printf("[%s", effective_memory_names[r_m]);
        }
        if (disp) {
          printf(" + %d], ", disp);
        } else {
          printf("], ");
        }
      }

      s16 data;
      if (!s) {
        data = w ? ReadS16(state) : ReadS8(state);
        printf("%d\n", data);
      } else {
        data = ReadU8(state);
        printf("%d\n", data);
      }
    } break;

    // accumulator (which is represented by the AX register in this case)
    case Op_MOV_ImmToReg: {
      u8 w = (byte & 0b00001000) >> 3;
      u8 reg = (byte & 0b00000111) >> 0;

      s8 *register_dst = register_names[w][reg];

      s16 data = w ? ReadU16(state) : ReadU8(state);

      printf("%s %s, %d\n", pattern_found->mnemonic, register_dst, data);

      cpu->registers[reg] = data;
    } break;

    case Op_MOV_MemToAccum: {
      u8 w = (byte & 0b00000001) >> 0;

      u16 addr = ReadU16(state);

      printf("%s %s, [%d]\n", pattern_found->mnemonic, w ? "ax" : "al", addr);
    } break;

    case Op_MOV_AccumToMem: {
      u8 w = (byte & 0b00000001) >> 0;

      u16 addr = ReadU16(state);

      printf("mov [%d], %s\n", addr, w ? "ax" : "al");
    } break;
    case Op_ADD_ImmToAccum:
    case Op_CMP_ImmToAccum:
    case Op_SUB_ImmToAccum: {
      u8 w = (byte & 0b00000001) >> 0;

      s16 data = w ? ReadS16(state) : ReadS8(state);

      printf("%s %s, %d\n", pattern_found->mnemonic, w ? "ax" : "al", data);
    } break;
    
    case Op_JE_JZ:
    case Op_JL_JNZ:
    case Op_JLE_JNG:
    case Op_JB_JNAE:
    case Op_JBE_JNA:
    case Op_JP_JPE:
    case Op_JO:
    case Op_JS:
    case Op_JNE_JNZ:
    case Op_JNL_JGE:
    case Op_JNLE_JG:
    case Op_JNB_JAE:
    case Op_JNBE_JA:
    case Op_JNP_JPO:
    case Op_JNO:
    case Op_JNS:
    case Op_LOOP:
    case Op_LOOPZ_LOOPE:
    case Op_LOOPNZ_LOOPNE:
    case Op_JCXZ : {
        s8 increment = ReadS8(state);
        printf("%s %d\n", pattern_found->mnemonic, increment);
    } break;

    default: {
      // printf("Unknown OpCode...\n");
    } break;
    }  
  }
  PrintCPUState(cpu);
  return 0;
}
