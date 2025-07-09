#include "cpu.h"
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define FLAG_Z 0x80
#define FLAG_N 0x40
#define FLAG_H 0x20
#define FLAG_C 0x10
#define FLAG_ALL 0xF0
#define Z_POS 7
#define N_POS 6
#define H_POS 5
#define C_POS 4

uint8_t fetch_byte(CPU *cpu) { return cpu->memory[cpu->PC++]; }

uint16_t fetch_word(CPU *cpu) {
  uint8_t low = fetch_byte(cpu);
  uint8_t high = fetch_byte(cpu);
  return high << 8 | low;
}

typedef void (*opcode_function)(CPU *cpu);
typedef void (*special_opcode_function)(CPU *cpu);
void prefix(CPU *cpu);

void not_implemented(CPU *cpu) {
  printf("Instruction not implemented: %02x\n\n", cpu->memory[cpu->PC - 1]);
  exit(EXIT_FAILURE);
}

uint8_t get_first_reg(uint16_t reg) { return reg >> 8; }
uint8_t get_last_reg(uint16_t reg) { return reg & 0x00FF; }

uint8_t get_z_flag(uint8_t f_reg) { return f_reg & 0x80 >> Z_POS; };
uint8_t get_n_flag(uint8_t f_reg) { return f_reg & 0x40 >> N_POS; };
uint8_t get_h_flag(uint8_t f_reg) { return f_reg & 0x20 >> H_POS; };
uint8_t get_c_flag(uint8_t f_reg) { return f_reg & 0x10 >> C_POS; };

void update_flags(CPU *cpu, uint8_t mask, uint8_t flags) {
  uint8_t f_reg = get_last_reg(cpu->AF);
  // Apply the bit masks to update selected flags only
  f_reg = (f_reg & ~mask) | (flags & mask);
  cpu->AF = cpu->AF & 0xFF00 | f_reg;
}

void nop(CPU *cpu) {};

void stop_n8(CPU *cpu) {};

void jr_nz_e8(CPU *cpu) {
  uint8_t f_reg = get_last_reg(cpu->AF);
  uint8_t z_flag = get_z_flag(f_reg);

  if (z_flag == 1) {
    int8_t value = (int8_t)fetch_byte(cpu);
    cpu->PC += value;
  }
};

void ld_sp_n16(CPU *cpu) {
  uint16_t value = fetch_word(cpu);
  cpu->SP = value;
};

void ld_a_n8(CPU *cpu) {
  uint8_t value = fetch_byte(cpu);
  uint8_t f_reg = get_last_reg(cpu->AF);
  cpu->AF = value << 8 | f_reg;
}

void ldh_a8_a(CPU *cpu) {
  uint8_t value = fetch_byte(cpu);
  cpu->memory[0xFF00 + value] = get_last_reg(cpu->AF);
}

void xor_a_a(CPU *cpu) {
  // Set Z flag and reset rest
  update_flags(cpu, FLAG_ALL, FLAG_Z);
}

void ld_hl_n16(CPU *cpu) {
  uint16_t value = fetch_word(cpu);
  cpu->HL = value;
}

void ld_hld_a(CPU *cpu) {
  uint8_t a_reg = get_first_reg(cpu->AF);
  cpu->memory[cpu->HL] = a_reg;
  cpu->HL--;
}

void bit_7_h(CPU *cpu) {
  uint8_t f_reg = get_last_reg(cpu->AF);
  uint8_t h_reg = get_first_reg(cpu->HL);

  uint8_t z_flag = (h_reg >> 7 & 1 ? FLAG_Z : 0x00);
  update_flags(cpu, FLAG_Z, z_flag);
}

void ei(CPU *cpu) {
  // Set IME flag at 0xFFFF
  cpu->memory[0xFFFF] = 1;
}

void ld_c_n8(CPU *cpu) {
  uint8_t value = fetch_byte(cpu);
  uint8_t b_reg = get_first_reg(cpu->BC);
  cpu->BC = b_reg | value;
}

void ldh_c_a(CPU *cpu) {
  uint8_t a_reg = get_first_reg(cpu->AF);
  uint8_t c_reg = get_last_reg(cpu->BC);
  cpu->memory[0xFF00 + c_reg] = a_reg;
}

void inc_c(CPU *cpu) {
  uint8_t b_reg = get_first_reg(cpu->BC);
  uint8_t c_reg = get_last_reg(cpu->BC);

  uint8_t h_flag = ((((c_reg & 0x0F) + 0x01) & 0x10) == 0x10) << H_POS;

  c_reg++;
  uint8_t z_flag = c_reg == 0 ? FLAG_Z : 0x00;

  update_flags(cpu, FLAG_H | FLAG_Z | FLAG_N, h_flag | z_flag);
  cpu->BC = b_reg << 8 | c_reg;
}

void inc_h(CPU *cpu) {
  uint8_t h_reg = get_first_reg(cpu->HL);
  uint8_t l_reg = get_last_reg(cpu->HL);

  uint8_t h_flag = ((((h_reg & 0x0F) + 0x01) & 0x10) == 0x10) << H_POS;

  h_reg++;
  uint8_t z_flag = h_reg == 0 ? FLAG_Z : 0x00;

  update_flags(cpu, FLAG_H | FLAG_Z | FLAG_N, h_flag | z_flag);
  cpu->BC = h_reg << 8 | l_reg;
}

void inc_d(CPU *cpu) {
  uint8_t d_reg = get_first_reg(cpu->DE);
  uint8_t e_reg = get_last_reg(cpu->DE);

  uint8_t h_flag = ((((d_reg & 0x0F) + 0x01) & 0x10) == 0x10) << H_POS;

  d_reg++;
  uint8_t z_flag = d_reg == 0 ? FLAG_Z : 0x00;

  update_flags(cpu, FLAG_H | FLAG_Z | FLAG_N, h_flag | z_flag);
  cpu->BC = d_reg << 8 | e_reg;
}

void inc_e(CPU *cpu) {
  uint8_t d_reg = get_first_reg(cpu->DE);
  uint8_t e_reg = get_last_reg(cpu->DE);

  uint8_t h_flag = ((((e_reg & 0x0F) + 0x01) & 0x10) == 0x10) << H_POS;

  e_reg++;
  uint8_t z_flag = e_reg == 0 ? FLAG_Z : 0x00;

  update_flags(cpu, FLAG_H | FLAG_Z | FLAG_N, h_flag | z_flag);
  cpu->BC = d_reg << 8 | e_reg;
}

void inc_b(CPU *cpu) {
  uint8_t b_reg = get_first_reg(cpu->BC);
  uint8_t c_reg = get_last_reg(cpu->BC);

  uint8_t h_flag = ((((b_reg & 0x0F) + 0x01) & 0x10) == 0x10) << H_POS;

  b_reg++;
  uint8_t z_flag = b_reg == 0 ? FLAG_Z : 0x00;

  update_flags(cpu, FLAG_H | FLAG_Z | FLAG_N, h_flag | z_flag);
  cpu->BC = c_reg << 8 | c_reg;
}

void inc_l(CPU *cpu) {
  uint8_t h_reg = get_first_reg(cpu->HL);
  uint8_t l_reg = get_last_reg(cpu->HL);

  uint8_t h_flag = ((((l_reg & 0x0F) + 0x01) & 0x10) == 0x10) << H_POS;

  l_reg++;
  uint8_t z_flag = l_reg == 0 ? FLAG_Z : 0x00;

  update_flags(cpu, FLAG_H | FLAG_Z | FLAG_N, h_flag | z_flag);
  cpu->BC = h_reg << 8 | l_reg;
}

void ld_hl_a(CPU *cpu) {
  uint8_t a_reg = get_first_reg(cpu->AF);
  cpu->memory[cpu->HL] = a_reg;
}

void ld_de_a(CPU *cpu) {
  uint16_t value = fetch_word(cpu);
  cpu->DE = value;
}

void ld_a_de(CPU *cpu) {
  uint8_t a_reg = get_first_reg(cpu->AF);
  a_reg = cpu->memory[cpu->DE];
  cpu->AF = a_reg << 8 | cpu->AF & 0x00FF;
}

void call_a16(CPU *cpu) {
  uint16_t value = fetch_word(cpu);
  uint16_t return_addr = cpu->PC;

  cpu->SP--;
  cpu->memory[cpu->SP] = return_addr >> 8; // high
  cpu->SP--;
  cpu->memory[cpu->SP] = return_addr & 0x00FF; // low

  // implicit n16 jump
  cpu->PC = value;
}

void ld_c_a(CPU *cpu) {
  uint8_t a_reg = get_first_reg(cpu->AF);
  uint8_t b_reg = get_first_reg(cpu->BC);
  cpu->BC = b_reg << 8 | a_reg;
}

void ld_b_n8(CPU *cpu) {
  uint8_t value = fetch_byte(cpu);
  uint8_t c_reg = get_last_reg(cpu->BC);
  cpu->BC = value << 8 | c_reg;
}

void rl_c(CPU *cpu) {
  uint8_t f_reg = get_last_reg(cpu->AF);
  uint8_t c_flag = get_c_flag(f_reg);

  uint8_t c_reg = get_last_reg(cpu->BC);
  uint8_t bit_7_c = c_reg >> 7;

  // rotate left
  c_reg = c_reg << 1;
  c_reg = c_reg & 0xFE | c_flag;
  c_flag = bit_7_c << C_POS;

  uint8_t z_flag = c_reg == 0 ? FLAG_Z : 0x00;
  update_flags(cpu, FLAG_Z | FLAG_H | FLAG_N | FLAG_C, z_flag | c_flag);

  cpu->BC = cpu->BC & 0xFF00 | c_reg;
}
void rla(CPU *cpu) {
  uint8_t f_reg = get_last_reg(cpu->AF);
  uint8_t c_flag = get_c_flag(f_reg);
  uint8_t a_reg = get_first_reg(cpu->AF);

  uint8_t bit_7_a = a_reg >> 7;

  // rotate left
  a_reg = a_reg << 1;
  a_reg = a_reg & 0xFE | c_flag;
  c_flag = bit_7_a << C_POS;

  update_flags(cpu, FLAG_Z | FLAG_H | FLAG_N | FLAG_C, c_flag);
  cpu->AF = f_reg << 8 | a_reg;
}

void pop_bc(CPU *cpu) {
  uint8_t low = cpu->memory[cpu->SP];
  cpu->SP++;
  uint8_t high = cpu->memory[cpu->SP];
  cpu->SP++;

  cpu->BC = high << 8 | low;
}

void dec_b(CPU *cpu) {
  uint8_t b_reg = get_first_reg(cpu->BC);
  uint8_t bit_4_b = b_reg & 0x10;
  b_reg--;
  uint8_t h_flag = (b_reg & 0x10) == bit_4_b ? 0x00 : FLAG_H;
  uint8_t z_flag = b_reg == 0 ? FLAG_Z : 0x00;
  update_flags(cpu, FLAG_Z | FLAG_H | FLAG_N, z_flag | h_flag | FLAG_N);
}

void push_bc(CPU *cpu) {
  cpu->SP--;
  cpu->memory[cpu->SP] = cpu->BC >> 8; // High
  cpu->SP--;
  cpu->memory[cpu->SP] = cpu->BC & 0x00FF; // Low
}

void push_af(CPU *cpu) {
  cpu->SP--;
  cpu->memory[cpu->SP] = cpu->AF >> 8; // High
  cpu->SP--;
  cpu->memory[cpu->SP] = cpu->AF & 0x00FF; // Low
}

void push_de(CPU *cpu) {
  cpu->SP--;
  cpu->memory[cpu->SP] = cpu->DE >> 8; // High
  cpu->SP--;
  cpu->memory[cpu->SP] = cpu->DE & 0x00FF; // Low
}

void push_hl(CPU *cpu) {
  cpu->SP--;
  cpu->memory[cpu->SP] = cpu->HL >> 8; // High
  cpu->SP--;
  cpu->memory[cpu->SP] = cpu->HL & 0x00FF; // Low
}

void ld_hli_a(CPU *cpu) {
  uint8_t a_reg = get_first_reg(cpu->AF);
  cpu->memory[cpu->HL] = a_reg;
  cpu->HL++;
}

void inc_hl(CPU *cpu) { cpu->HL++; }
void inc_bc(CPU *cpu) { cpu->BC++; }
void inc_de(CPU *cpu) { cpu->DE++; }
void inc_sp(CPU *cpu) { cpu->SP++; }

void dec_hl(CPU *cpu) { cpu->HL--; }
void dec_bc(CPU *cpu) { cpu->BC--; }
void dec_de(CPU *cpu) { cpu->DE--; }
void dec_sp(CPU *cpu) { cpu->SP--; }

void ret(CPU *cpu) {
  uint8_t low = cpu->memory[cpu->SP];
  cpu->SP++;
  uint8_t high = cpu->memory[cpu->SP];
  cpu->SP++;
  cpu->PC = high << 8 | low;
}

special_opcode_function special_opcode_table[256] = {
    [0x00] = not_implemented, //
    [0x01] = not_implemented, //
    [0x02] = not_implemented, //
    [0x03] = not_implemented, //
    [0x04] = not_implemented, //
    [0x05] = not_implemented, //
    [0x06] = not_implemented, //
    [0x07] = not_implemented, //
    [0x08] = not_implemented, //
    [0x09] = not_implemented, //
    [0x0A] = not_implemented, //
    [0x0B] = not_implemented, //
    [0x0C] = not_implemented, //
    [0x0D] = not_implemented, //
    [0x0E] = not_implemented, //
    [0x0F] = not_implemented, //
    [0x10] = not_implemented, //
    [0x11] = rl_c,            //
    [0x12] = not_implemented, //
    [0x13] = not_implemented, //
    [0x14] = not_implemented, //
    [0x15] = not_implemented, //
    [0x16] = not_implemented, //
    [0x17] = not_implemented, //
    [0x18] = not_implemented, //
    [0x19] = not_implemented, //
    [0x1A] = not_implemented, //
    [0x1B] = not_implemented, //
    [0x1C] = not_implemented, //
    [0x1D] = not_implemented, //
    [0x1E] = not_implemented, //
    [0x1F] = not_implemented, //
    [0x20] = not_implemented, //
    [0x21] = not_implemented, //
    [0x22] = not_implemented, //
    [0x23] = not_implemented, //
    [0x24] = not_implemented, //
    [0x25] = not_implemented, //
    [0x26] = not_implemented, //
    [0x27] = not_implemented, //
    [0x28] = not_implemented, //
    [0x29] = not_implemented, //
    [0x2A] = not_implemented, //
    [0x2B] = not_implemented, //
    [0x2C] = not_implemented, //
    [0x2D] = not_implemented, //
    [0x2E] = not_implemented, //
    [0x2F] = not_implemented, //
    [0x30] = not_implemented, //
    [0x31] = not_implemented, //
    [0x32] = not_implemented, //
    [0x33] = not_implemented, //
    [0x34] = not_implemented, //
    [0x35] = not_implemented, //
    [0x36] = not_implemented, //
    [0x37] = not_implemented, //
    [0x38] = not_implemented, //
    [0x39] = not_implemented, //
    [0x3A] = not_implemented, //
    [0x3B] = not_implemented, //
    [0x3C] = not_implemented, //
    [0x3D] = not_implemented, //
    [0x3E] = not_implemented, //
    [0x3F] = not_implemented, //
    [0x40] = not_implemented, //
    [0x41] = not_implemented, //
    [0x42] = not_implemented, //
    [0x43] = not_implemented, //
    [0x44] = not_implemented, //
    [0x45] = not_implemented, //
    [0x46] = not_implemented, //
    [0x47] = not_implemented, //
    [0x48] = not_implemented, //
    [0x49] = not_implemented, //
    [0x4A] = not_implemented, //
    [0x4B] = not_implemented, //
    [0x4C] = not_implemented, //
    [0x4D] = not_implemented, //
    [0x4E] = not_implemented, //
    [0x4F] = not_implemented, //
    [0x50] = not_implemented, //
    [0x51] = not_implemented, //
    [0x52] = not_implemented, //
    [0x53] = not_implemented, //
    [0x54] = not_implemented, //
    [0x55] = not_implemented, //
    [0x56] = not_implemented, //
    [0x57] = not_implemented, //
    [0x58] = not_implemented, //
    [0x59] = not_implemented, //
    [0x5A] = not_implemented, //
    [0x5B] = not_implemented, //
    [0x5C] = not_implemented, //
    [0x5D] = not_implemented, //
    [0x5E] = not_implemented, //
    [0x5F] = not_implemented, //
    [0x60] = not_implemented, //
    [0x61] = not_implemented, //
    [0x62] = not_implemented, //
    [0x63] = not_implemented, //
    [0x64] = not_implemented, //
    [0x65] = not_implemented, //
    [0x66] = not_implemented, //
    [0x67] = not_implemented, //
    [0x68] = not_implemented, //
    [0x69] = not_implemented, //
    [0x6A] = not_implemented, //
    [0x6B] = not_implemented, //
    [0x6C] = not_implemented, //
    [0x6D] = not_implemented, //
    [0x6E] = not_implemented, //
    [0x6F] = not_implemented, //
    [0x70] = not_implemented, //
    [0x71] = not_implemented, //
    [0x72] = not_implemented, //
    [0x73] = not_implemented, //
    [0x74] = not_implemented, //
    [0x75] = not_implemented, //
    [0x76] = not_implemented, //
    [0x77] = not_implemented, //
    [0x78] = not_implemented, //
    [0x79] = not_implemented, //
    [0x7A] = not_implemented, //
    [0x7B] = not_implemented, //
    [0x7C] = bit_7_h,         //
    [0x7D] = not_implemented, //
    [0x7E] = not_implemented, //
    [0x7F] = not_implemented, //
    [0x80] = not_implemented, //
    [0x81] = not_implemented, //
    [0x82] = not_implemented, //
    [0x83] = not_implemented, //
    [0x84] = not_implemented, //
    [0x85] = not_implemented, //
    [0x86] = not_implemented, //
    [0x87] = not_implemented, //
    [0x88] = not_implemented, //
    [0x89] = not_implemented, //
    [0x8A] = not_implemented, //
    [0x8B] = not_implemented, //
    [0x8C] = not_implemented, //
    [0x8D] = not_implemented, //
    [0x8E] = not_implemented, //
    [0x8F] = not_implemented, //
    [0x90] = not_implemented, //
    [0x91] = not_implemented, //
    [0x92] = not_implemented, //
    [0x93] = not_implemented, //
    [0x94] = not_implemented, //
    [0x95] = not_implemented, //
    [0x96] = not_implemented, //
    [0x97] = not_implemented, //
    [0x98] = not_implemented, //
    [0x99] = not_implemented, //
    [0x9A] = not_implemented, //
    [0x9B] = not_implemented, //
    [0x9C] = not_implemented, //
    [0x9D] = not_implemented, //
    [0x9E] = not_implemented, //
    [0x9F] = not_implemented, //
    [0xA0] = not_implemented, //
    [0xA1] = not_implemented, //
    [0xA2] = not_implemented, //
    [0xA3] = not_implemented, //
    [0xA4] = not_implemented, //
    [0xA5] = not_implemented, //
    [0xA6] = not_implemented, //
    [0xA7] = not_implemented, //
    [0xA8] = not_implemented, //
    [0xA9] = not_implemented, //
    [0xAA] = not_implemented, //
    [0xAB] = not_implemented, //
    [0xAC] = not_implemented, //
    [0xAD] = not_implemented, //
    [0xAE] = not_implemented, //
    [0xAF] = push_af,         //
    [0xB0] = not_implemented, //
    [0xB1] = not_implemented, //
    [0xB2] = not_implemented, //
    [0xB3] = not_implemented, //
    [0xB4] = not_implemented, //
    [0xB5] = not_implemented, //
    [0xB6] = not_implemented, //
    [0xB7] = not_implemented, //
    [0xB8] = not_implemented, //
    [0xB9] = not_implemented, //
    [0xBA] = not_implemented, //
    [0xBB] = not_implemented, //
    [0xBC] = not_implemented, //
    [0xBD] = not_implemented, //
    [0xBE] = not_implemented, //
    [0xBF] = not_implemented, //
    [0xC0] = not_implemented, //
    [0xC1] = not_implemented, //
    [0xC2] = not_implemented, //
    [0xC3] = not_implemented, //
    [0xC4] = not_implemented, //
    [0xC5] = not_implemented, //
    [0xC6] = not_implemented, //
    [0xC7] = not_implemented, //
    [0xC8] = not_implemented, //
    [0xC9] = not_implemented, //
    [0xCA] = not_implemented, //
    [0xCB] = not_implemented, //
    [0xCC] = not_implemented, //
    [0xCD] = not_implemented, //
    [0xCE] = not_implemented, //
    [0xCF] = not_implemented, //
    [0xD0] = not_implemented, //
    [0xD1] = not_implemented, //
    [0xD2] = not_implemented, //
    [0xD3] = not_implemented, //
    [0xD4] = not_implemented, //
    [0xD5] = not_implemented, //
    [0xD6] = not_implemented, //
    [0xD7] = not_implemented, //
    [0xD8] = not_implemented, //
    [0xD9] = not_implemented, //
    [0xDA] = not_implemented, //
    [0xDB] = not_implemented, //
    [0xDC] = not_implemented, //
    [0xDD] = not_implemented, //
    [0xDE] = not_implemented, //
    [0xDF] = not_implemented, //
    [0xE0] = not_implemented, //
    [0xE1] = not_implemented, //
    [0xE2] = not_implemented, //
    [0xE3] = not_implemented, //
    [0xE4] = not_implemented, //
    [0xE5] = not_implemented, //
    [0xE6] = not_implemented, //
    [0xE7] = not_implemented, //
    [0xE8] = not_implemented, //
    [0xE9] = not_implemented, //
    [0xEA] = not_implemented, //
    [0xEB] = not_implemented, //
    [0xEC] = not_implemented, //
    [0xED] = not_implemented, //
    [0xEE] = not_implemented, //
    [0xEF] = not_implemented, //
    [0xF0] = not_implemented, //
    [0xF1] = not_implemented, //
    [0xF2] = not_implemented, //
    [0xF3] = not_implemented, //
    [0xF4] = not_implemented, //
    [0xF5] = not_implemented, //
    [0xF6] = not_implemented, //
    [0xF7] = not_implemented, //
    [0xF8] = not_implemented, //
    [0xF9] = not_implemented, //
    [0xFA] = not_implemented, //
    [0xFB] = not_implemented, //
    [0xFC] = not_implemented, //
    [0xFD] = not_implemented, //
    [0xFE] = not_implemented, //
    [0xFF] = not_implemented, //
};

opcode_function opcode_table[256] = {
    [0x00] = nop,             //
    [0x01] = not_implemented, //
    [0x02] = not_implemented, //
    [0x03] = not_implemented, //
    [0x04] = not_implemented, //
    [0x05] = dec_b,           //
    [0x06] = ld_b_n8,         //
    [0x07] = not_implemented, //
    [0x08] = not_implemented, //
    [0x09] = not_implemented, //
    [0x0A] = not_implemented, //
    [0x0B] = not_implemented, //
    [0x0C] = inc_c,           //
    [0x0D] = not_implemented, //
    [0x0E] = ld_c_n8,         //
    [0x0F] = not_implemented, //
    [0x10] = stop_n8,         //
    [0x11] = ld_de_a,         //
    [0x12] = not_implemented, //
    [0x13] = not_implemented, //
    [0x14] = not_implemented, //
    [0x15] = not_implemented, //
    [0x16] = not_implemented, //
    [0x17] = rla,             //
    [0x18] = not_implemented, //
    [0x19] = not_implemented, //
    [0x1A] = ld_a_de,         //
    [0x1B] = not_implemented, //
    [0x1C] = not_implemented, //
    [0x1D] = not_implemented, //
    [0x1E] = not_implemented, //
    [0x1F] = not_implemented, //
    [0x20] = jr_nz_e8,        //
    [0x21] = ld_hl_n16,       //
    [0x22] = ld_hli_a,        //
    [0x23] = inc_hl,          //
    [0x24] = not_implemented, //
    [0x25] = not_implemented, //
    [0x26] = not_implemented, //
    [0x27] = not_implemented, //
    [0x28] = not_implemented, //
    [0x29] = not_implemented, //
    [0x2A] = not_implemented, //
    [0x2B] = not_implemented, //
    [0x2C] = not_implemented, //
    [0x2D] = not_implemented, //
    [0x2E] = not_implemented, //
    [0x2F] = not_implemented, //
    [0x30] = not_implemented, //
    [0x31] = ld_sp_n16,       //
    [0x32] = ld_hld_a,        //
    [0x33] = not_implemented, //
    [0x34] = not_implemented, //
    [0x35] = not_implemented, //
    [0x36] = not_implemented, //
    [0x37] = not_implemented, //
    [0x38] = not_implemented, //
    [0x39] = not_implemented, //
    [0x3A] = not_implemented, //
    [0x3B] = not_implemented, //
    [0x3C] = not_implemented, //
    [0x3D] = not_implemented, //
    [0x3E] = ld_a_n8,         //
    [0x3F] = not_implemented, //
    [0x40] = not_implemented, //
    [0x41] = not_implemented, //
    [0x42] = not_implemented, //
    [0x43] = not_implemented, //
    [0x44] = not_implemented, //
    [0x45] = not_implemented, //
    [0x46] = not_implemented, //
    [0x47] = not_implemented, //
    [0x48] = not_implemented, //
    [0x49] = not_implemented, //
    [0x4A] = not_implemented, //
    [0x4B] = not_implemented, //
    [0x4C] = not_implemented, //
    [0x4D] = not_implemented, //
    [0x4E] = not_implemented, //
    [0x4F] = ld_c_a,          //
    [0x50] = not_implemented, //
    [0x51] = not_implemented, //
    [0x52] = not_implemented, //
    [0x53] = not_implemented, //
    [0x54] = not_implemented, //
    [0x55] = not_implemented, //
    [0x56] = not_implemented, //
    [0x57] = not_implemented, //
    [0x58] = not_implemented, //
    [0x59] = not_implemented, //
    [0x5A] = not_implemented, //
    [0x5B] = not_implemented, //
    [0x5C] = not_implemented, //
    [0x5D] = not_implemented, //
    [0x5E] = not_implemented, //
    [0x5F] = not_implemented, //
    [0x60] = not_implemented, //
    [0x61] = not_implemented, //
    [0x62] = not_implemented, //
    [0x63] = not_implemented, //
    [0x64] = not_implemented, //
    [0x65] = not_implemented, //
    [0x66] = not_implemented, //
    [0x67] = not_implemented, //
    [0x68] = not_implemented, //
    [0x69] = not_implemented, //
    [0x6A] = not_implemented, //
    [0x6B] = not_implemented, //
    [0x6C] = not_implemented, //
    [0x6D] = not_implemented, //
    [0x6E] = not_implemented, //
    [0x6F] = not_implemented, //
    [0x70] = not_implemented, //
    [0x71] = not_implemented, //
    [0x72] = not_implemented, //
    [0x73] = not_implemented, //
    [0x74] = not_implemented, //
    [0x75] = not_implemented, //
    [0x76] = not_implemented, //
    [0x77] = ld_hl_a,         //
    [0x78] = not_implemented, //
    [0x79] = not_implemented, //
    [0x7A] = not_implemented, //
    [0x7B] = not_implemented, //
    [0x7C] = not_implemented, //
    [0x7D] = not_implemented, //
    [0x7E] = not_implemented, //
    [0x7F] = not_implemented, //
    [0x80] = not_implemented, //
    [0x81] = not_implemented, //
    [0x82] = not_implemented, //
    [0x83] = not_implemented, //
    [0x84] = not_implemented, //
    [0x85] = not_implemented, //
    [0x86] = not_implemented, //
    [0x87] = not_implemented, //
    [0x88] = not_implemented, //
    [0x89] = not_implemented, //
    [0x8A] = not_implemented, //
    [0x8B] = not_implemented, //
    [0x8C] = not_implemented, //
    [0x8D] = not_implemented, //
    [0x8E] = not_implemented, //
    [0x8F] = not_implemented, //
    [0x90] = not_implemented, //
    [0x91] = not_implemented, //
    [0x92] = not_implemented, //
    [0x93] = not_implemented, //
    [0x94] = not_implemented, //
    [0x95] = not_implemented, //
    [0x96] = not_implemented, //
    [0x97] = not_implemented, //
    [0x98] = not_implemented, //
    [0x99] = not_implemented, //
    [0x9A] = not_implemented, //
    [0x9B] = not_implemented, //
    [0x9C] = not_implemented, //
    [0x9D] = not_implemented, //
    [0x9E] = not_implemented, //
    [0x9F] = not_implemented, //
    [0xA0] = not_implemented, //
    [0xA1] = not_implemented, //
    [0xA2] = not_implemented, //
    [0xA3] = not_implemented, //
    [0xA4] = not_implemented, //
    [0xA5] = not_implemented, //
    [0xA6] = not_implemented, //
    [0xA7] = not_implemented, //
    [0xA8] = not_implemented, //
    [0xA9] = not_implemented, //
    [0xAA] = not_implemented, //
    [0xAB] = not_implemented, //
    [0xAC] = not_implemented, //
    [0xAD] = not_implemented, //
    [0xAE] = not_implemented, //
    [0xAF] = xor_a_a,         //
    [0xB0] = not_implemented, //
    [0xB1] = not_implemented, //
    [0xB2] = not_implemented, //
    [0xB3] = not_implemented, //
    [0xB4] = not_implemented, //
    [0xB5] = not_implemented, //
    [0xB6] = not_implemented, //
    [0xB7] = not_implemented, //
    [0xB8] = not_implemented, //
    [0xB9] = not_implemented, //
    [0xBA] = not_implemented, //
    [0xBB] = not_implemented, //
    [0xBC] = not_implemented, //
    [0xBD] = not_implemented, //
    [0xBE] = not_implemented, //
    [0xBF] = not_implemented, //
    [0xC0] = not_implemented, //
    [0xC1] = pop_bc,          //
    [0xC2] = not_implemented, //
    [0xC3] = not_implemented, //
    [0xC4] = not_implemented, //
    [0xC5] = push_bc,         //
    [0xC6] = not_implemented, //
    [0xC7] = not_implemented, //
    [0xC8] = not_implemented, //
    [0xC9] = ret,             //
    [0xCA] = not_implemented, //
    [0xCB] = prefix,          //
    [0xCC] = not_implemented, //
    [0xCD] = call_a16,        //
    [0xCE] = not_implemented, //
    [0xCF] = not_implemented, //
    [0xD0] = not_implemented, //
    [0xD1] = not_implemented, //
    [0xD2] = not_implemented, //
    [0xD3] = not_implemented, //
    [0xD4] = not_implemented, //
    [0xD5] = push_de,         //
    [0xD6] = not_implemented, //
    [0xD7] = not_implemented, //
    [0xD8] = not_implemented, //
    [0xD9] = not_implemented, //
    [0xDA] = not_implemented, //
    [0xDB] = not_implemented, //
    [0xDC] = not_implemented, //
    [0xDD] = not_implemented, //
    [0xDE] = not_implemented, //
    [0xDF] = not_implemented, //
    [0xE0] = ldh_a8_a,        //
    [0xE1] = not_implemented, //
    [0xE2] = ldh_c_a,         //
    [0xE3] = not_implemented, //
    [0xE4] = not_implemented, //
    [0xE5] = push_hl,         //
    [0xE6] = not_implemented, //
    [0xE7] = not_implemented, //
    [0xE8] = not_implemented, //
    [0xE9] = not_implemented, //
    [0xEA] = not_implemented, //
    [0xEB] = not_implemented, //
    [0xEC] = not_implemented, //
    [0xED] = not_implemented, //
    [0xEE] = not_implemented, //
    [0xEF] = not_implemented, //
    [0xF0] = not_implemented, //
    [0xF1] = not_implemented, //
    [0xF2] = not_implemented, //
    [0xF3] = not_implemented, //
    [0xF4] = not_implemented, //
    [0xF5] = push_af,         //
    [0xF6] = not_implemented, //
    [0xF7] = not_implemented, //
    [0xF8] = not_implemented, //
    [0xF9] = not_implemented, //
    [0xFA] = not_implemented, //
    [0xFB] = ei,              //
    [0xFC] = not_implemented, //
    [0xFD] = not_implemented, //
    [0xFE] = not_implemented, //
    [0xFF] = not_implemented, //
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
