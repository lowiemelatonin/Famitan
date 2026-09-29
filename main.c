#include <stdio.h>
#include "cpu.h"

int main(void){
    CPU_6502 cpu;

    cpu_reset(&cpu);

    cpu.X = 10;
    cpu.A = 5;
    memory[0x8000] = 0xE8;

    uint8_t opcode = cpu_fetch(&cpu);
    cpu_execute(&cpu, opcode);

    printf("Opcode: %02X\n", opcode);
    printf("PC: %04X\n", cpu.PC);
    printf("X: %02X\n", cpu.X);
    printf("Y: %02X\n", cpu.Y);
    printf("A: %02X\n", cpu.A);

    return 0;
}