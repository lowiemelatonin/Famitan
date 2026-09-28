#include "cpu.h"

uint8_t memory[0x10000];

void set_flag(CPU_6502 *cpu, uint8_t flag, int value){
    if(value){
        cpu->Status |= flag;
    } else {
        cpu->Status &= ~flag;
    }
}

int get_flag(CPU_6502 *cpu, uint8_t flag){
    return (cpu->Status & flag) != 0;
}

void cpu_reset(CPU_6502 *cpu){
    cpu->A = 0;
    cpu->X = 0;
    cpu->Y = 0;
    cpu->SP = 0xFD;
    cpu->Status = FLAG_I | FLAG_U;
    cpu->PC = 0x8000;
}

uint8_t cpu_read(uint16_t address){
    return memory[address];
}

void cpu_write(uint16_t address, uint8_t value){
    memory[address] = value;
}

uint8_t cpu_fetch(CPU_6502 *cpu){
    uint8_t value = cpu_read(cpu->PC);
    cpu->PC++;
    return value;
}