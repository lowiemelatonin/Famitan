#ifndef CPU_H
#define CPU_H

#include <stdint.h>

#define FLAG_C 0x01
#define FLAG_Z 0x02
#define FLAG_I 0x04
#define FLAG_D 0x08
#define FLAG_B 0x10
#define FLAG_U 0x20
#define FLAG_V 0x40
#define FLAG_N 0x80

typedef struct {
    uint16_t PC;
    uint8_t SP;
    uint8_t A;
    uint8_t X;
    uint8_t Y;
    uint8_t Status;
} CPU_6502;

extern uint8_t memory[0x10000];

void set_flag(CPU_6502 *cpu, uint8_t flag, int value);
int get_flag(CPU_6502 *cpu, uint8_t flag);

void cpu_reset(CPU_6502 *cpu);

uint8_t cpu_read(uint16_t address);
void cpu_write(uint16_t address, uint8_t value);

uint8_t cpu_fetch(CPU_6502 *cpu);

#endif