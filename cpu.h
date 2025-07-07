#ifndef CPU_NEOSAHADEO
#define CPU_NEOSAHADEO

#include <inttypes.h>
typedef struct CPU {
  // General Memory
  uint8_t *memory;
  uint8_t *vram;

  // Registers
  uint16_t BC;
  uint16_t DE;
  uint16_t HL;

  uint16_t AF; // Accumulator

  uint16_t SP; // Stack pointer
  uint16_t PC; // Program counter

} CPU;

void initialize_cpu(CPU *cpu, uint8_t *memory);
void step(CPU *cpu);
void destroy_cpu(CPU *cpu);

#endif
