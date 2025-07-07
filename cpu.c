#include "cpu.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

uint8_t fetch_byte(CPU *cpu) { return cpu->memory[cpu->PC++]; }

uint16_t fetch_word(CPU *cpu) {
  uint8_t low = fetch_byte(cpu);
  uint8_t high = fetch_byte(cpu);
  return high << 8 | low;
}

typedef void (*opcode_function)(CPU *cpu);
typedef void (*special_opcode_function)(CPU *cpu);
void prefix(CPU *cpu);

void nop(CPU *cpu) { cpu->PC++; };

void stop_n8(CPU *cpu) {};

void jr_nz_e8(CPU *cpu) {
  if ((cpu->HL >> 15) == 1) {
    int8_t value = (int8_t)fetch_byte(cpu);
    cpu->PC += value;
  }
};

void jr_nc_e8(CPU *cpu) {

};

void ld_sp_n16(CPU *cpu) {
  uint16_t value = fetch_word(cpu);
  cpu->SP = value;
};

void ld_a_n8(CPU *cpu) {
  uint16_t value = fetch_byte(cpu) << 8;
  uint16_t f_reg = cpu->AF << 8;
  cpu->AF = value | f_reg;
}

void ldh_a8_a(CPU *cpu) {
  uint8_t value = fetch_byte(cpu);
  cpu->memory[0xFF00 + value] = (uint8_t)(cpu->AF >> 8);
}

void xor_a_a(CPU *cpu) {
  // Set Z flag and reset rest
  uint16_t a_reg = cpu->AF & 0xFF00;
  cpu->AF = a_reg | 0x80;
}

void ld_hl_n16(CPU *cpu) {
  uint16_t value = fetch_word(cpu);
  cpu->HL = value;
}

void ld_hld_a(CPU *cpu) {
  uint8_t a_reg = (uint8_t)(cpu->AF >> 8);
  cpu->memory[cpu->HL] = a_reg;
  cpu->HL--;
}

void bit_7_h(CPU *cpu) {
  uint8_t bit7 = (cpu->HL >> 15) & 1;
  uint8_t z_flag = (bit7 == 0) ? 0x80 : 0x00;
  uint8_t f_reg = cpu->AF & 0x00FF;
  uint8_t c_flag = f_reg & 0x10;
  f_reg = z_flag | 0x00 | 0x20 | c_flag;
  cpu->AF = (cpu->AF & 0xFF00) | f_reg;
}

special_opcode_function special_opcode_table[256] = {
    [0x00] = nop,     //
    [0x01] = nop,     //
    [0x02] = nop,     //
    [0x03] = nop,     //
    [0x04] = nop,     //
    [0x05] = nop,     //
    [0x06] = nop,     //
    [0x07] = nop,     //
    [0x08] = nop,     //
    [0x09] = nop,     //
    [0x0A] = nop,     //
    [0x0B] = nop,     //
    [0x0C] = nop,     //
    [0x0D] = nop,     //
    [0x0E] = nop,     //
    [0x0F] = nop,     //
    [0x10] = nop,     //
    [0x11] = nop,     //
    [0x12] = nop,     //
    [0x13] = nop,     //
    [0x14] = nop,     //
    [0x15] = nop,     //
    [0x16] = nop,     //
    [0x17] = nop,     //
    [0x18] = nop,     //
    [0x19] = nop,     //
    [0x1A] = nop,     //
    [0x1B] = nop,     //
    [0x1C] = nop,     //
    [0x1D] = nop,     //
    [0x1E] = nop,     //
    [0x1F] = nop,     //
    [0x20] = nop,     //
    [0x21] = nop,     //
    [0x22] = nop,     //
    [0x23] = nop,     //
    [0x24] = nop,     //
    [0x25] = nop,     //
    [0x26] = nop,     //
    [0x27] = nop,     //
    [0x28] = nop,     //
    [0x29] = nop,     //
    [0x2A] = nop,     //
    [0x2B] = nop,     //
    [0x2C] = nop,     //
    [0x2D] = nop,     //
    [0x2E] = nop,     //
    [0x2F] = nop,     //
    [0x30] = nop,     //
    [0x31] = nop,     //
    [0x32] = nop,     //
    [0x33] = nop,     //
    [0x34] = nop,     //
    [0x35] = nop,     //
    [0x36] = nop,     //
    [0x37] = nop,     //
    [0x38] = nop,     //
    [0x39] = nop,     //
    [0x3A] = nop,     //
    [0x3B] = nop,     //
    [0x3C] = nop,     //
    [0x3D] = nop,     //
    [0x3E] = nop,     //
    [0x3F] = nop,     //
    [0x40] = nop,     //
    [0x41] = nop,     //
    [0x42] = nop,     //
    [0x43] = nop,     //
    [0x44] = nop,     //
    [0x45] = nop,     //
    [0x46] = nop,     //
    [0x47] = nop,     //
    [0x48] = nop,     //
    [0x49] = nop,     //
    [0x4A] = nop,     //
    [0x4B] = nop,     //
    [0x4C] = nop,     //
    [0x4D] = nop,     //
    [0x4E] = nop,     //
    [0x4F] = nop,     //
    [0x50] = nop,     //
    [0x51] = nop,     //
    [0x52] = nop,     //
    [0x53] = nop,     //
    [0x54] = nop,     //
    [0x55] = nop,     //
    [0x56] = nop,     //
    [0x57] = nop,     //
    [0x58] = nop,     //
    [0x59] = nop,     //
    [0x5A] = nop,     //
    [0x5B] = nop,     //
    [0x5C] = nop,     //
    [0x5D] = nop,     //
    [0x5E] = nop,     //
    [0x5F] = nop,     //
    [0x60] = nop,     //
    [0x61] = nop,     //
    [0x62] = nop,     //
    [0x63] = nop,     //
    [0x64] = nop,     //
    [0x65] = nop,     //
    [0x66] = nop,     //
    [0x67] = nop,     //
    [0x68] = nop,     //
    [0x69] = nop,     //
    [0x6A] = nop,     //
    [0x6B] = nop,     //
    [0x6C] = nop,     //
    [0x6D] = nop,     //
    [0x6E] = nop,     //
    [0x6F] = nop,     //
    [0x70] = nop,     //
    [0x71] = nop,     //
    [0x72] = nop,     //
    [0x73] = nop,     //
    [0x74] = nop,     //
    [0x75] = nop,     //
    [0x76] = nop,     //
    [0x77] = nop,     //
    [0x78] = nop,     //
    [0x79] = nop,     //
    [0x7A] = nop,     //
    [0x7B] = nop,     //
    [0x7C] = bit_7_h, //
    [0x7D] = nop,     //
    [0x7E] = nop,     //
    [0x7F] = nop,     //
    [0x80] = nop,     //
    [0x81] = nop,     //
    [0x82] = nop,     //
    [0x83] = nop,     //
    [0x84] = nop,     //
    [0x85] = nop,     //
    [0x86] = nop,     //
    [0x87] = nop,     //
    [0x88] = nop,     //
    [0x89] = nop,     //
    [0x8A] = nop,     //
    [0x8B] = nop,     //
    [0x8C] = nop,     //
    [0x8D] = nop,     //
    [0x8E] = nop,     //
    [0x8F] = nop,     //
    [0x90] = nop,     //
    [0x91] = nop,     //
    [0x92] = nop,     //
    [0x93] = nop,     //
    [0x94] = nop,     //
    [0x95] = nop,     //
    [0x96] = nop,     //
    [0x97] = nop,     //
    [0x98] = nop,     //
    [0x99] = nop,     //
    [0x9A] = nop,     //
    [0x9B] = nop,     //
    [0x9C] = nop,     //
    [0x9D] = nop,     //
    [0x9E] = nop,     //
    [0x9F] = nop,     //
    [0xA0] = nop,     //
    [0xA1] = nop,     //
    [0xA2] = nop,     //
    [0xA3] = nop,     //
    [0xA4] = nop,     //
    [0xA5] = nop,     //
    [0xA6] = nop,     //
    [0xA7] = nop,     //
    [0xA8] = nop,     //
    [0xA9] = nop,     //
    [0xAA] = nop,     //
    [0xAB] = nop,     //
    [0xAC] = nop,     //
    [0xAD] = nop,     //
    [0xAE] = nop,     //
    [0xAF] = nop,     //
    [0xB0] = nop,     //
    [0xB1] = nop,     //
    [0xB2] = nop,     //
    [0xB3] = nop,     //
    [0xB4] = nop,     //
    [0xB5] = nop,     //
    [0xB6] = nop,     //
    [0xB7] = nop,     //
    [0xB8] = nop,     //
    [0xB9] = nop,     //
    [0xBA] = nop,     //
    [0xBB] = nop,     //
    [0xBC] = nop,     //
    [0xBD] = nop,     //
    [0xBE] = nop,     //
    [0xBF] = nop,     //
    [0xC0] = nop,     //
    [0xC1] = nop,     //
    [0xC2] = nop,     //
    [0xC3] = nop,     //
    [0xC4] = nop,     //
    [0xC5] = nop,     //
    [0xC6] = nop,     //
    [0xC7] = nop,     //
    [0xC8] = nop,     //
    [0xC9] = nop,     //
    [0xCA] = nop,     //
    [0xCB] = nop,     //
    [0xCC] = nop,     //
    [0xCD] = nop,     //
    [0xCE] = nop,     //
    [0xCF] = nop,     //
    [0xD0] = nop,     //
    [0xD1] = nop,     //
    [0xD2] = nop,     //
    [0xD3] = nop,     //
    [0xD4] = nop,     //
    [0xD5] = nop,     //
    [0xD6] = nop,     //
    [0xD7] = nop,     //
    [0xD8] = nop,     //
    [0xD9] = nop,     //
    [0xDA] = nop,     //
    [0xDB] = nop,     //
    [0xDC] = nop,     //
    [0xDD] = nop,     //
    [0xDE] = nop,     //
    [0xDF] = nop,     //
    [0xE0] = nop,     //
    [0xE1] = nop,     //
    [0xE2] = nop,     //
    [0xE3] = nop,     //
    [0xE4] = nop,     //
    [0xE5] = nop,     //
    [0xE6] = nop,     //
    [0xE7] = nop,     //
    [0xE8] = nop,     //
    [0xE9] = nop,     //
    [0xEA] = nop,     //
    [0xEB] = nop,     //
    [0xEC] = nop,     //
    [0xED] = nop,     //
    [0xEE] = nop,     //
    [0xEF] = nop,     //
    [0xF0] = nop,     //
    [0xF1] = nop,     //
    [0xF2] = nop,     //
    [0xF3] = nop,     //
    [0xF4] = nop,     //
    [0xF5] = nop,     //
    [0xF6] = nop,     //
    [0xF7] = nop,     //
    [0xF8] = nop,     //
    [0xF9] = nop,     //
    [0xFA] = nop,     //
    [0xFB] = nop,     //
    [0xFC] = nop,     //
    [0xFD] = nop,     //
    [0xFE] = nop,     //
    [0xFF] = nop,     //

};

opcode_function opcode_table[256] = {
    [0x00] = nop,       //
    [0x01] = nop,       //
    [0x02] = nop,       //
    [0x03] = nop,       //
    [0x04] = nop,       //
    [0x05] = nop,       //
    [0x06] = nop,       //
    [0x07] = nop,       //
    [0x08] = nop,       //
    [0x09] = nop,       //
    [0x0A] = nop,       //
    [0x0B] = nop,       //
    [0x0C] = nop,       //
    [0x0D] = nop,       //
    [0x0E] = nop,       //
    [0x0F] = nop,       //
    [0x10] = stop_n8,   //
    [0x11] = nop,       //
    [0x12] = nop,       //
    [0x13] = nop,       //
    [0x14] = nop,       //
    [0x15] = nop,       //
    [0x16] = nop,       //
    [0x17] = nop,       //
    [0x18] = nop,       //
    [0x19] = nop,       //
    [0x1A] = nop,       //
    [0x1B] = nop,       //
    [0x1C] = nop,       //
    [0x1D] = nop,       //
    [0x1E] = nop,       //
    [0x1F] = nop,       //
    [0x20] = jr_nz_e8,  //
    [0x21] = ld_hl_n16, //
    [0x22] = nop,       //
    [0x23] = nop,       //
    [0x24] = nop,       //
    [0x25] = nop,       //
    [0x26] = nop,       //
    [0x27] = nop,       //
    [0x28] = nop,       //
    [0x29] = nop,       //
    [0x2A] = nop,       //
    [0x2B] = nop,       //
    [0x2C] = nop,       //
    [0x2D] = nop,       //
    [0x2E] = nop,       //
    [0x2F] = nop,       //
    [0x30] = jr_nc_e8,  //
    [0x31] = ld_sp_n16, //
    [0x32] = ld_hld_a,  //
    [0x33] = nop,       //
    [0x34] = nop,       //
    [0x35] = nop,       //
    [0x36] = nop,       //
    [0x37] = nop,       //
    [0x38] = nop,       //
    [0x39] = nop,       //
    [0x3A] = nop,       //
    [0x3B] = nop,       //
    [0x3C] = nop,       //
    [0x3D] = nop,       //
    [0x3E] = ld_a_n8,   //
    [0x3F] = nop,       //
    [0x40] = nop,       //
    [0x41] = nop,       //
    [0x42] = nop,       //
    [0x43] = nop,       //
    [0x44] = nop,       //
    [0x45] = nop,       //
    [0x46] = nop,       //
    [0x47] = nop,       //
    [0x48] = nop,       //
    [0x49] = nop,       //
    [0x4A] = nop,       //
    [0x4B] = nop,       //
    [0x4C] = nop,       //
    [0x4D] = nop,       //
    [0x4E] = nop,       //
    [0x4F] = nop,       //
    [0x50] = nop,       //
    [0x51] = nop,       //
    [0x52] = nop,       //
    [0x53] = nop,       //
    [0x54] = nop,       //
    [0x55] = nop,       //
    [0x56] = nop,       //
    [0x57] = nop,       //
    [0x58] = nop,       //
    [0x59] = nop,       //
    [0x5A] = nop,       //
    [0x5B] = nop,       //
    [0x5C] = nop,       //
    [0x5D] = nop,       //
    [0x5E] = nop,       //
    [0x5F] = nop,       //
    [0x60] = nop,       //
    [0x61] = nop,       //
    [0x62] = nop,       //
    [0x63] = nop,       //
    [0x64] = nop,       //
    [0x65] = nop,       //
    [0x66] = nop,       //
    [0x67] = nop,       //
    [0x68] = nop,       //
    [0x69] = nop,       //
    [0x6A] = nop,       //
    [0x6B] = nop,       //
    [0x6C] = nop,       //
    [0x6D] = nop,       //
    [0x6E] = nop,       //
    [0x6F] = nop,       //
    [0x70] = nop,       //
    [0x71] = nop,       //
    [0x72] = nop,       //
    [0x73] = nop,       //
    [0x74] = nop,       //
    [0x75] = nop,       //
    [0x76] = nop,       //
    [0x77] = nop,       //
    [0x78] = nop,       //
    [0x79] = nop,       //
    [0x7A] = nop,       //
    [0x7B] = nop,       //
    [0x7C] = nop,       //
    [0x7D] = nop,       //
    [0x7E] = nop,       //
    [0x7F] = nop,       //
    [0x80] = nop,       //
    [0x81] = nop,       //
    [0x82] = nop,       //
    [0x83] = nop,       //
    [0x84] = nop,       //
    [0x85] = nop,       //
    [0x86] = nop,       //
    [0x87] = nop,       //
    [0x88] = nop,       //
    [0x89] = nop,       //
    [0x8A] = nop,       //
    [0x8B] = nop,       //
    [0x8C] = nop,       //
    [0x8D] = nop,       //
    [0x8E] = nop,       //
    [0x8F] = nop,       //
    [0x90] = nop,       //
    [0x91] = nop,       //
    [0x92] = nop,       //
    [0x93] = nop,       //
    [0x94] = nop,       //
    [0x95] = nop,       //
    [0x96] = nop,       //
    [0x97] = nop,       //
    [0x98] = nop,       //
    [0x99] = nop,       //
    [0x9A] = nop,       //
    [0x9B] = nop,       //
    [0x9C] = nop,       //
    [0x9D] = nop,       //
    [0x9E] = nop,       //
    [0x9F] = nop,       //
    [0xA0] = nop,       //
    [0xA1] = nop,       //
    [0xA2] = nop,       //
    [0xA3] = nop,       //
    [0xA4] = nop,       //
    [0xA5] = nop,       //
    [0xA6] = nop,       //
    [0xA7] = nop,       //
    [0xA8] = nop,       //
    [0xA9] = nop,       //
    [0xAA] = nop,       //
    [0xAB] = nop,       //
    [0xAC] = nop,       //
    [0xAD] = nop,       //
    [0xAE] = nop,       //
    [0xAF] = xor_a_a,   //
    [0xB0] = nop,       //
    [0xB1] = nop,       //
    [0xB2] = nop,       //
    [0xB3] = nop,       //
    [0xB4] = nop,       //
    [0xB5] = nop,       //
    [0xB6] = nop,       //
    [0xB7] = nop,       //
    [0xB8] = nop,       //
    [0xB9] = nop,       //
    [0xBA] = nop,       //
    [0xBB] = nop,       //
    [0xBC] = nop,       //
    [0xBD] = nop,       //
    [0xBE] = nop,       //
    [0xBF] = nop,       //
    [0xC0] = nop,       //
    [0xC1] = nop,       //
    [0xC2] = nop,       //
    [0xC3] = nop,       //
    [0xC4] = nop,       //
    [0xC5] = nop,       //
    [0xC6] = nop,       //
    [0xC7] = nop,       //
    [0xC8] = nop,       //
    [0xC9] = nop,       //
    [0xCA] = nop,       //
    [0xCB] = prefix,    //
    [0xCC] = nop,       //
    [0xCD] = nop,       //
    [0xCE] = nop,       //
    [0xCF] = nop,       //
    [0xD0] = nop,       //
    [0xD1] = nop,       //
    [0xD2] = nop,       //
    [0xD3] = nop,       //
    [0xD4] = nop,       //
    [0xD5] = nop,       //
    [0xD6] = nop,       //
    [0xD7] = nop,       //
    [0xD8] = nop,       //
    [0xD9] = nop,       //
    [0xDA] = nop,       //
    [0xDB] = nop,       //
    [0xDC] = nop,       //
    [0xDD] = nop,       //
    [0xDE] = nop,       //
    [0xDF] = nop,       //
    [0xE0] = ldh_a8_a,  //
    [0xE1] = nop,       //
    [0xE2] = nop,       //
    [0xE3] = nop,       //
    [0xE4] = nop,       //
    [0xE5] = nop,       //
    [0xE6] = nop,       //
    [0xE7] = nop,       //
    [0xE8] = nop,       //
    [0xE9] = nop,       //
    [0xEA] = nop,       //
    [0xEB] = nop,       //
    [0xEC] = nop,       //
    [0xED] = nop,       //
    [0xEE] = nop,       //
    [0xEF] = nop,       //
    [0xF0] = nop,       //
    [0xF1] = nop,       //
    [0xF2] = nop,       //
    [0xF3] = nop,       //
    [0xF4] = nop,       //
    [0xF5] = nop,       //
    [0xF6] = nop,       //
    [0xF7] = nop,       //
    [0xF8] = nop,       //
    [0xF9] = nop,       //
    [0xFA] = nop,       //
    [0xFB] = nop,       //
    [0xFC] = nop,       //
    [0xFD] = nop,       //
    [0xFE] = nop,       //
    [0xFF] = nop,       //
};

void initialize_cpu(CPU *cpu, uint8_t *memory) {
  cpu->PC = 0;
  cpu->memory = memory;
  if (cpu->memory == NULL) {
    perror("Failed to allocate memory for the cpu.");
    exit(EXIT_FAILURE);
  }
}

void step(CPU *cpu) {
  uint8_t instruction = fetch_byte(cpu);
  printf("Executing -> %02x\n", instruction);
  opcode_table[instruction](cpu);
}

void prefix(CPU *cpu) {
  uint8_t instruction = fetch_byte(cpu);
  printf("Executing(S) -> %02x\n", instruction);
  special_opcode_table[instruction](cpu);
}
void destroy_cpu(CPU *cpu) { free(cpu->memory); }
