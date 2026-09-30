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

uint16_t addr_absolute(CPU_6502 *cpu){
    uint8_t low = cpu_fetch(cpu);
    uint8_t high = cpu_fetch(cpu);

    return low | (high << 8);
}

uint16_t addr_absolute_x(CPU_6502 *cpu){
    return addr_absolute(cpu) + cpu->X;
}

uint16_t addr_absolute_y(CPU_6502 *cpu){
    return addr_absolute(cpu) + cpu->Y;
}

uint8_t addr_zero_page(CPU_6502 *cpu){
    return cpu_fetch(cpu);
}

uint8_t addr_zero_page_x(CPU_6502 *cpu){
    return cpu_fetch(cpu) + cpu->X;
}

uint8_t addr_zero_page_y(CPU_6502 *cpu){
    return cpu_fetch(cpu) + cpu->Y;
}

uint16_t addr_indirect(CPU_6502 *cpu){
    uint16_t pointer = addr_absolute(cpu);

    uint8_t low = cpu_read(pointer);
    uint8_t high = cpu_read(pointer + 1);

    return low | (high << 8);
}

uint16_t addr_indirect_x(CPU_6502 *cpu){
    uint8_t address = cpu_fetch(cpu);
    address += cpu->X;

    uint8_t low = cpu_read(address);
    uint8_t high = cpu_read((uint8_t)(address + 1));

    return low | (high << 8);
}

uint16_t addr_indirect_y(CPU_6502 *cpu){
    uint8_t address = cpu_fetch(cpu);

    uint8_t low = cpu_read(address);
    uint8_t high = cpu_read((uint8_t)(address + 1));

    uint16_t target = low | (high << 8);

    return target + cpu->Y;
}

void cpu_execute(CPU_6502 *cpu, uint8_t opcode){
    switch(opcode){
        case 0xE8: // INX
            cpu->X++;
            break;
        case 0x0A: // ASL A
            cpu->A <<= 1;
            break;
        case 0xD0: // BNE Relative
            {
                int8_t offset = cpu_fetch(cpu);
                if(!get_flag(cpu, FLAG_Z)){
                    cpu->PC += offset;
                }
                break;
            }
        case 0xA9: // LDA #immediate
            cpu->A = cpu_fetch(cpu);
            break;
        case 0xA5: // LDA Zero Page
            cpu->A = cpu_read(addr_zero_page(cpu));
            break;
        case 0xB5: // LDA Zero Page, X
            cpu->A = cpu_read(addr_zero_page_x(cpu));
            break;
        case 0xAD: // LDA Absolute
            cpu->A = cpu_read(addr_absolute(cpu));
            break;
        case 0xBD: // LDA Absolute, X
            cpu->A = cpu_read(addr_absolute_x(cpu));
            break;
        case 0xB9: // LDA Absolute, Y
            cpu->A = cpu_read(addr_absolute_y(cpu));
            break;
        case 0xA1: // LDA Indirect, X
            cpu->A = cpu_read(addr_indirect_x(cpu));
            break;
        case 0xB1: // LDA Indirect, Y
            cpu->A = cpu_read(addr_indirect_y(cpu));
            break;
        case 0x6C: // JMP Indirect
            cpu->PC = addr_indirect(cpu);
            break;
        case 0xA2: // LDX #immediate
            cpu->X = cpu_fetch(cpu);
            break;
        case 0xA6: // LDX Zero Page
            cpu->X = cpu_read(addr_zero_page(cpu));
            break;
        case 0xB6: // LDX Zero Page, Y
            cpu->X = cpu_read(addr_zero_page_y(cpu));
            break;
        case 0xAE: // LDX Absolute
            cpu->X = cpu_read(addr_absolute(cpu));
            break;
        case 0xBE: // LDX Absolute, Y
            cpu->X = cpu_read(addr_absolute_y(cpu));
            break;
        case 0xA0: // LDY #immediate
            cpu->Y = cpu_fetch(cpu);
            break;
        case 0xA4: // LDY Zero Page
            cpu->Y = cpu_read(addr_zero_page(cpu));
            break;
        case 0xB4: // LDY Zero Page, X
            cpu->Y = cpu_read(addr_zero_page_x(cpu));
            break;
        case 0xAC: // LDY Absolute
            cpu->Y = cpu_read(addr_absolute(cpu));
            break;
        case 0xBC: // LDY Absolute, X
            cpu->Y = cpu_read(addr_absolute_x(cpu));
            break;
        default:
            break;
    }
}