#include <stdio.h>
#include "cpu.h"

int main(void){
    CPU_6502 cpu;

    cpu_reset(&cpu);

    memory[0x8000] = 0xA9;

    uint8_t opcode = cpu_fetch(&cpu);

    printf("Opcode: %02X\n", opcode);
    printf("PC: %04X\n", cpu.PC);

    return 0;
}