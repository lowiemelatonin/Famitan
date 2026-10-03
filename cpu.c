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

void set_zn(CPU_6502 *cpu, uint8_t value){
    set_flag(cpu, FLAG_Z, value == 0);
    set_flag(cpu, FLAG_N, value & 0x80);
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

void adc(CPU_6502 *cpu, uint8_t value){
    uint16_t result = cpu->A + value + get_flag(cpu, FLAG_C);

    set_flag(cpu, FLAG_C, result > 0xFF);
    set_flag(cpu, FLAG_V, (~(cpu->A ^ value) & (cpu->A ^ result) & 0x80) != 0);

    cpu->A = (uint8_t)result;
    set_zn(cpu, cpu->A);
}

void sbc(CPU_6502 *cpu, uint8_t value){
    uint16_t result = cpu->A + (uint8_t)~value + get_flag(cpu, FLAG_C);

    set_flag(cpu, FLAG_C, result & 0x100);
    set_flag(cpu, FLAG_V, ((cpu->A ^ result) & (~value ^ result) & 0x80) != 0);
    
    cpu->A = (uint8_t)result;
    set_zn(cpu, cpu->A);
}

void and_op(CPU_6502 *cpu, uint8_t value){
    cpu->A &= value;
    set_zn(cpu, cpu->A);
}

void ora(CPU_6502 *cpu, uint8_t value){
    cpu->A |= value;
    set_zn(cpu, cpu->A);
}

void eor(CPU_6502 *cpu, uint8_t value){
    cpu->A ^= value;
    set_zn(cpu, cpu->A);
}

void cpu_execute(CPU_6502 *cpu, uint8_t opcode){
    switch(opcode){
        case 0xE8: // INX
            cpu->X++;
            set_zn(cpu, cpu->X);
            break;
        case 0x0A: // ASL A
            set_flag(cpu, FLAG_C, cpu->A & 0x80);
            cpu->A <<= 1;
            set_zn(cpu, cpu->A);
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
            set_zn(cpu, cpu->A);
            break;
        case 0xA5: // LDA Zero Page
            cpu->A = cpu_read(addr_zero_page(cpu));
            set_zn(cpu, cpu->A);
            break;
        case 0xB5: // LDA Zero Page, X
            cpu->A = cpu_read(addr_zero_page_x(cpu));
            set_zn(cpu, cpu->A);
            break;
        case 0xAD: // LDA Absolute
            cpu->A = cpu_read(addr_absolute(cpu));
            set_zn(cpu, cpu->A);
            break;
        case 0xBD: // LDA Absolute, X
            cpu->A = cpu_read(addr_absolute_x(cpu));
            set_zn(cpu, cpu->A);
            break;
        case 0xB9: // LDA Absolute, Y
            cpu->A = cpu_read(addr_absolute_y(cpu));
            set_zn(cpu, cpu->A);
            break;
        case 0xA1: // LDA Indirect, X
            cpu->A = cpu_read(addr_indirect_x(cpu));
            set_zn(cpu, cpu->A);
            break;
        case 0xB1: // LDA Indirect, Y
            cpu->A = cpu_read(addr_indirect_y(cpu));
            set_zn(cpu, cpu->A);
            break;
        case 0x6C: // JMP Indirect
            cpu->PC = addr_indirect(cpu);
            break;
        case 0xA2: // LDX #immediate
            cpu->X = cpu_fetch(cpu);
            set_zn(cpu, cpu->X);
            break;
        case 0xA6: // LDX Zero Page
            cpu->X = cpu_read(addr_zero_page(cpu));
            set_zn(cpu, cpu->X);
            break;
        case 0xB6: // LDX Zero Page, Y
            cpu->X = cpu_read(addr_zero_page_y(cpu));
            set_zn(cpu, cpu->X);
            break;
        case 0xAE: // LDX Absolute
            cpu->X = cpu_read(addr_absolute(cpu));
            set_zn(cpu, cpu->X);
            break;
        case 0xBE: // LDX Absolute, Y
            cpu->X = cpu_read(addr_absolute_y(cpu));
            set_zn(cpu, cpu->X);
            break;
        case 0xA0: // LDY #immediate
            cpu->Y = cpu_fetch(cpu);
            set_zn(cpu, cpu->Y);
            break;
        case 0xA4: // LDY Zero Page
            cpu->Y = cpu_read(addr_zero_page(cpu));
            set_zn(cpu, cpu->Y);
            break;
        case 0xB4: // LDY Zero Page, X
            cpu->Y = cpu_read(addr_zero_page_x(cpu));
            set_zn(cpu, cpu->Y);
            break;
        case 0xAC: // LDY Absolute
            cpu->Y = cpu_read(addr_absolute(cpu));
            set_zn(cpu, cpu->Y);
            break;
        case 0xBC: // LDY Absolute, X
            cpu->Y = cpu_read(addr_absolute_x(cpu));
            set_zn(cpu, cpu->Y);
            break;
        case 0x85: // STA Zero Page
            cpu_write(addr_zero_page(cpu), cpu->A);
            break;
        case 0x95: // STA Zero Page, X
            cpu_write(addr_zero_page_x(cpu), cpu->A);
            break;
        case 0x8D: // STA Absolute
            cpu_write(addr_absolute(cpu), cpu->A);
            break;
        case 0x9D: // STA Absolute, X
            cpu_write(addr_absolute_x(cpu), cpu->A);
            break;
        case 0x99: // STA Absolute, Y
            cpu_write(addr_absolute_y(cpu), cpu->A);
            break;
        case 0x81: // STA Indirect, X
            cpu_write(addr_indirect_x(cpu), cpu->A);
            break;
        case 0x91: // STA Indirect, Y
            cpu_write(addr_indirect_y(cpu), cpu->A);
            break;
        case 0x86: // STX Zero Page
            cpu_write(addr_zero_page(cpu), cpu->X);
            break;
        case 0x96: // STX Zero Page, Y
            cpu_write(addr_zero_page_y(cpu), cpu->X);
            break;
        case 0x8E: // STX Absolute
            cpu_write(addr_absolute(cpu), cpu->X);
            break;
        case 0x84: // STY Zero Page
            cpu_write(addr_zero_page(cpu), cpu->Y);
            break;
        case 0x94: // STY Zero Page, X
            cpu_write(addr_zero_page_x(cpu), cpu->Y);
            break;
        case 0x8C: // STY Absolute
            cpu_write(addr_absolute(cpu), cpu->Y);
            break;
        case 0x69: // ADC #immediate
            adc(cpu, cpu_fetch(cpu));
            break;
        case 0x65: // ADC Zero Page
            adc(cpu, cpu_read(addr_zero_page(cpu)));
            break;
        case 0x75: // ADC Zero Page, X
            adc(cpu, cpu_read(addr_zero_page_x(cpu)));
            break;
        case 0x6D: // ADC Absolute
            adc(cpu, cpu_read(addr_absolute(cpu)));
            break;
        case 0x7D: // ADC Absolute, X
            adc(cpu, cpu_read(addr_absolute_x(cpu)));
            break;
        case 0x79: // ADC Absolute, Y
            adc(cpu, cpu_read(addr_absolute_y(cpu)));
            break;
        case 0x61: // ADC Indirect, X
            adc(cpu, cpu_read(addr_indirect_x(cpu)));
            break;
        case 0x71: // ADC Indirect, Y
            adc(cpu, cpu_read(addr_indirect_y(cpu)));
            break;
        case 0xE9: // SBC #immediate
            sbc(cpu, cpu_fetch(cpu));
            break;
        case 0xE5: // SBC Zero Page
            sbc(cpu, cpu_read(addr_zero_page(cpu)));
            break;
        case 0xF5: // SBC Zero Page, X
            sbc(cpu, cpu_read(addr_zero_page_x(cpu)));
            break;
        case 0xED: // SBC Absolute
            sbc(cpu, cpu_read(addr_absolute(cpu)));
            break;
        case 0xFD: // SBC Absolute, X
            sbc(cpu, cpu_read(addr_absolute_x(cpu)));
            break;
        case 0xF9: // SBC Absolute, Y
            sbc(cpu, cpu_read(addr_absolute_y(cpu)));
            break;
        case 0xE1: // SBC Indirect, X
            sbc(cpu, cpu_read(addr_indirect_x(cpu)));
            break;
        case 0xF1: // SBC Indirect, Y
            sbc(cpu, cpu_read(addr_indirect_y(cpu)));
            break;
        case 0x29: // AND #immediate
            and_op(cpu, cpu_fetch(cpu));
            break;
        case 0x25: // AND Zero Page
            and_op(cpu, cpu_read(addr_zero_page(cpu)));
            break;
        case 0x35: // AND Zero Page, X
            and_op(cpu, cpu_read(addr_zero_page_x(cpu)));
            break;
        case 0x2D: // AND Absolute
            and_op(cpu, cpu_read(addr_absolute(cpu)));
            break;
        case 0x3D: // AND Absolute, X
            and_op(cpu, cpu_read(addr_absolute_x(cpu)));
            break;
        case 0x39: // AND Absolute, Y
            and_op(cpu, cpu_read(addr_absolute_y(cpu)));
            break;
        case 0x21: // AND Indirect, X
            and_op(cpu, cpu_read(addr_indirect_x(cpu)));
            break;
        case 0x31: // AND Indirect, Y
            and_op(cpu, cpu_read(addr_indirect_y(cpu)));
            break;
        case 0x09: // ORA #immediate
            ora(cpu, cpu_fetch(cpu));
            break;
        case 0x05: // ORA Zero Page
            ora(cpu, cpu_read(addr_zero_page(cpu)));
            break;
        case 0x15: // ORA Zero Page, X
            ora(cpu, cpu_read(addr_zero_page_x(cpu)));
            break;
        case 0x0D: // ORA Absolute
            ora(cpu, cpu_read(addr_absolute(cpu)));
            break;
        case 0x1D: // ORA Absolute, X
            ora(cpu, cpu_read(addr_absolute_x(cpu)));
            break;
        case 0x19: // ORA Absolute, Y
            ora(cpu, cpu_read(addr_absolute_y(cpu)));
            break;
        case 0x01: // ORA Indirect, X
            ora(cpu, cpu_read(addr_indirect_x(cpu)));
            break;
        case 0x11: // ORA Indirect, Y
            ora(cpu, cpu_read(addr_indirect_y(cpu)));
            break;
        case 0x49: // EOR #immediate
            eor(cpu, cpu_fetch(cpu));
            break;
        case 0x45: // EOR Zero Page
            eor(cpu, cpu_read(addr_zero_page(cpu)));
            break;
        case 0x55: // EOR Zero Page, X
            eor(cpu, cpu_read(addr_zero_page_x(cpu)));
            break;
        case 0x4D: // EOR Absolute
            eor(cpu, cpu_read(addr_absolute(cpu)));
            break;
        case 0x5D: // EOR Absolute, X
            eor(cpu, cpu_read(addr_absolute_x(cpu)));
            break;
        case 0x59: // EOR Absolute, Y
            eor(cpu, cpu_read(addr_absolute_y(cpu)));
            break;
        case 0x41: // EOR Indirect, X
            eor(cpu, cpu_read(addr_indirect_x(cpu)));
            break;
        case 0x51: // EOR Indirect, Y
            eor(cpu, cpu_read(addr_indirect_y(cpu)));
            break;
        default:
            break;
    }
}