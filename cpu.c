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

void cpu_execute(CPU_6502 *cpu, uint8_t opcode){
    switch(opcode){
        case 0xE8: // INX
            cpu->X++;
            break;
        case 0x0A: // ASL A
            cpu->A <<= 1;
            break;
        case 0xA9: // LDA #immediate
            cpu->A = cpu_fetch(cpu);
            break;
        case 0xA5: // LDA Zero Page
            {
                uint8_t address = cpu_fetch(cpu);
                cpu->A = cpu_read(address);
                break;
            }
        case 0xB5: // LDA Zero Page, X
            {
                uint8_t address = cpu_fetch(cpu);
                address += cpu->X;
                cpu->A = cpu_read(address);
                break;
            }
        case 0xB6: // LDX Zero Page, Y
            {
                uint8_t address = cpu_fetch(cpu);
                address += cpu->Y;
                cpu->X = cpu_read(address);
                break;
            }
        case 0xD0: // BNE Relative
            {
                int8_t offset = cpu_fetch(cpu);
                if(!get_flag(cpu, FLAG_Z)){
                    cpu->PC += offset;
                }
                break;
            }
        case 0xAD: // LDA Absolute
            {
                uint8_t low = cpu_fetch(cpu);
                uint8_t high = cpu_fetch(cpu);

                uint16_t address = low | (high << 8);

                cpu->A = cpu_read(address);
                break;
            }
        case 0xBD: // LDA Absolute, X
            {
                uint8_t low = cpu_fetch(cpu);
                uint8_t high = cpu_fetch(cpu);

                uint16_t address = low | (high << 8);
                address += cpu->X;

                cpu->A = cpu_read(address);
                break;
            }
        case 0xB9: // LDA Absolute, Y
            {
                uint8_t low = cpu_fetch(cpu);
                uint8_t high = cpu_fetch(cpu);

                uint16_t address = low | (high << 8);
                address += cpu->Y;

                cpu->A = cpu_read(address);
                break;
            }
        case 0x6C: // JMP Indirect
            {
                uint8_t low = cpu_fetch(cpu);
                uint8_t high = cpu_fetch(cpu);

                uint16_t pointer = low | (high << 8);
                
                uint8_t target_low = cpu_read(pointer);
                uint8_t target_high = cpu_read(pointer + 1);

                cpu->PC = target_low | (target_high << 8);
                break;
            }
        case 0xA1: // LDA Indirect, X
            {
                uint8_t address = cpu_fetch(cpu);

                address += cpu->X;

                uint8_t low = cpu_read(address);
                uint8_t high = cpu_read((uint8_t)(address + 1));

                uint16_t target = low | (high << 8);

                cpu->A = cpu_read(target);
                break;
            }
        case 0xB1: // LDA Indirect, Y
            {
                uint8_t address = cpu_fetch(cpu);

                uint8_t low = cpu_read(address);
                uint8_t high = cpu_read((uint8_t)(address + 1));

                uint16_t target = low | (high << 8);

                target += cpu->Y;

                cpu->A = cpu_read(target);
                break;
            }
        default:
            break;
    }
}