/* CPU8008 - Intel 8008 Emulator, with emulated videocontroller
 * By Yasin Morsli
*/
using namespace std;

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include "cpu8008.h"

cpu8008::cpu8008(){
    reset();
}

void cpu8008::reset(){
    for(uint8_t i = 0; i < elements(pc); i++)   pc[i] = 0;
    pc_p = 0;
    for(uint8_t i = 0; i < elements(regs); i++) regs[i] = 0;
    carry = zero = sign = parity = halt = false;
}

void cpu8008::step(){
    last_read.clear();
    last_write.clear();
    uint8_t _instr = readByte(pc[pc_p]);
    last_instruction = decode_instruction(_instr);
    pc[pc_p] = (pc[pc_p] + 1) % sizeof(mem);
    uint32_t _cycles = execute(_instr);
    update_counters(_cycles);
}

void cpu8008::step_out(){
    
}

cpu8008::instruction_t cpu8008::decode_instruction(uint8_t _instr){
    // HALT on x"00", x"FE" or x"FF":
    if((_instr == 0x00) || (_instr == 0xFE) || (_instr == 0xFF))
        return HLT;
    // Lrr:
    else if(((_instr & 0xC0) == 0xC0) && ((_instr & 0x38) != 0x38) && ((_instr & 0x07) != 0x07))
        return Lrr;
    // LrM:
    else if(((_instr & 0xC0) == 0xC0) && ((_instr & 0x38) != 0x38) && ((_instr & 0x07) == 0x07))
        return LrM;
    // LMr:
    else if(((_instr & 0xC0) == 0xC0) && ((_instr & 0x38) == 0x38) && ((_instr & 0x07) != 0x07))
        return LMr;
    // LrI:
    else if(((_instr & 0xC0) == 0x00) && ((_instr & 0x38) != 0x38) && ((_instr & 0x07) == 0x06))
        return LrI;
    // LMI:
    else if(((_instr & 0xC0) == 0x00) && ((_instr & 0x38) == 0x38) && ((_instr & 0x07) == 0x06))
        return LMI;
    // INr:
    else if(((_instr & 0xC0) == 0x00) && (((_instr & 0x38) != 0x00) || (((_instr & 0x38)) != 0x38)) && ((_instr & 0x07) == 0x00))
        return INr;
    // DCr:
    else if(((_instr & 0xC0) == 0x00) && (((_instr & 0x38) != 0x00) || (((_instr & 0x38)) != 0x38)) && ((_instr & 0x07) == 0x01))
        return DCr;
    // ALUr:
    else if(((_instr & 0xC0) == 0x80) && ((_instr & 0x07) != 0x07))
        return ALUr;
    // ALUM:
    else if(((_instr & 0xC0) == 0x80) && ((_instr & 0x07) == 0x07))
        return ALUM;
    // ALUI:
    else if(((_instr & 0xC0) == 0x00) && ((_instr & 0x07) == 0x04))
        return ALUI;
    // RLC:
    else if(((_instr & 0xC0) == 0x00) && ((_instr & 0x38) == 0x00) && ((_instr & 0x07) == 0x02))
        return RLC;
    // RRC:
    else if(((_instr & 0xC0) == 0x00) && ((_instr & 0x38) == 0x08) && ((_instr & 0x07) == 0x02))
        return RRC;
    // RAL:
    else if(((_instr & 0xC0) == 0x00) && ((_instr & 0x38) == 0x10) && ((_instr & 0x07) == 0x02))
        return RAL;
    // RAR:
    else if(((_instr & 0xC0) == 0x00) && ((_instr & 0x38) == 0x18) && ((_instr & 0x07) == 0x02))
        return RAR;
    // JMP:
    else if(((_instr & 0xC0) == 0x40) && ((_instr & 0x07) == 0x04))
        return JMP;
    // JFc:
    else if(((_instr & 0xC0) == 0x40) && ((_instr & 0x20) == 0x00) && ((_instr & 0x07) == 0x00))
        return JFc;
    // JTc:
    else if(((_instr & 0xC0) == 0x40) && ((_instr & 0x20) == 0x20) && ((_instr & 0x07) == 0x00))
        return JTc;
    // CAL:
    else if(((_instr & 0xC0) == 0x40)and ((_instr & 0x07) == 0x06))
        return CAL;
    // CFc:
    else if(((_instr & 0xC0) == 0x40) && ((_instr & 0x20) == 0x00) && ((_instr & 0x07) == 0x02))
        return CFc;
    // CTc:
    else if(((_instr & 0xC0) == 0x40) && ((_instr & 0x20) == 0x20) && ((_instr & 0x07) == 0x02))
        return CTc;
    // RET:
    else if(((_instr & 0xC0) == 0x00)and ((_instr & 0x07) == 0x07))
        return RET;
    // RFc:
    else if(((_instr & 0xC0) == 0x00) && ((_instr & 0x20) == 0x00) && ((_instr & 0x07) == 0x03))
        return RFc;
    // RTc:
    else if(((_instr & 0xC0) == 0x00) && ((_instr & 0x20) == 0x20) && ((_instr & 0x07) == 0x03))
        return RTc;
    // RST:
    else if(((_instr & 0xC0) == 0x00) && ((_instr & 0x07) == 0x05))
        return RST;
    // INP:
    else if(((_instr & 0xC0) == 0x40) && ((_instr & 0x30) == 0x00) && ((_instr & 0x01) == 0x01))
        return INP;
    // OUTP:
    else if(((_instr & 0xC0) == 0x40) && ((_instr & 0x30) != 0x00) && ((_instr & 0x01) == 0x01))
        return OUTP;
    else
        return UNKNOWN;
}

uint8_t cpu8008::execute(uint8_t _instr){
    int16_t _result = 0;
    uint8_t _parity = 0;
    uint8_t _cycles = cycle_counts[decode_instruction(_instr)];
    
    switch(decode_instruction(_instr)){
        case Lrr:
            regs[dst_reg(_instr)] = regs[src_reg(_instr)];
            break;
        case LrM:
            regs[dst_reg(_instr)] = readByte(mem_addr());
            break;
        case LMr:
            writeByte(mem_addr(), regs[src_reg(_instr)]);
            break;
        case LrI:
            regs[dst_reg(_instr)] = readByte(pc[pc_p]++);
            break;
        case LMI:
            writeByte(mem_addr(), readByte(pc[pc_p]++));
            break;
        case INr:
            regs[dst_reg(_instr)]++;
            zero = (regs[dst_reg(_instr)] == 0);
            sign = (regs[dst_reg(_instr)] & 0x80);
            for(int i = 0; i < 8; i++) _parity ^= ((regs[dst_reg(_instr)] >> i) & 1);
            parity = _parity;
            break;
        case DCr:
            regs[dst_reg(_instr)]--;
            zero = (regs[dst_reg(_instr)] == 0);
            sign = (regs[dst_reg(_instr)] & 0x80);
            for(int i = 0; i < 8; i++) _parity ^= ((regs[dst_reg(_instr)] >> i) & 1);
            parity = _parity;
            break;
        case ALUr:
            switch(alu_op(_instr)){
                case ADD:
                    _result = regs[REGS_A] + regs[src_reg(_instr)];
                    regs[REGS_A] = _result;
                    carry = (_result > 0x00FF);
                    zero = (_result == 0);
                    sign = (_result & 0x80);
                    for(int i = 0; i < 8; i++) _parity ^= ((_result >> i) & 1);
                    parity = _parity;
                    break;
                case ADC:
                    _result = regs[REGS_A] + regs[src_reg(_instr)] + ((carry) ? 1 : 0);
                    regs[REGS_A] = _result;
                    carry = (_result > 0x00FF);
                    zero = (_result == 0);
                    sign = (_result & 0x80);
                    for(int i = 0; i < 8; i++) _parity ^= ((_result >> i) & 1);
                    parity = _parity;
                    break;
                case SUB:
                    _result = regs[REGS_A] - regs[src_reg(_instr)];
                    regs[REGS_A] = _result;
                    carry = (_result < 0x0000);
                    zero = (_result == 0);
                    sign = (_result & 0x80);
                    for(int i = 0; i < 8; i++) _parity ^= ((_result >> i) & 1);
                    parity = _parity;
                    break;
                case SBC:
                    _result = regs[REGS_A] - regs[src_reg(_instr)] - ((carry) ? 1 : 0);
                    regs[REGS_A] = _result;
                    carry = (_result < 0x0000);
                    zero = (_result == 0);
                    sign = (_result & 0x80);
                    for(int i = 0; i < 8; i++) _parity ^= ((_result >> i) & 1);
                    parity = _parity;
                    break;
                case AND:
                    carry = false;
                    regs[REGS_A] &= regs[src_reg(_instr)];
                    zero = (regs[REGS_A] == 0);
                    sign = (regs[REGS_A] & 0x80);
                    for(int i = 0; i < 8; i++) _parity ^= ((_result >> i) & 1);
                    parity = _parity;
                    break;
                case XOR:
                    carry = false;
                    regs[REGS_A] ^= regs[src_reg(_instr)];
                    zero = (regs[REGS_A] == 0);
                    sign = (regs[REGS_A] & 0x80);
                    for(int i = 0; i < 8; i++) _parity ^= ((_result >> i) & 1);
                    parity = _parity;
                    break;
                case OR:
                    carry = false;
                    regs[REGS_A] |= regs[src_reg(_instr)];
                    zero = (regs[REGS_A] == 0);
                    sign = (regs[REGS_A] & 0x80);
                    for(int i = 0; i < 8; i++) _parity ^= ((_result >> i) & 1);
                    parity = _parity;
                    break;
                case CP:
                    _result = regs[REGS_A] - regs[src_reg(_instr)];
                    carry = (_result < 0x0000);
                    zero = (_result == 0);
                    sign = (_result & 0x80);
                    for(int i = 0; i < 8; i++) _parity ^= ((_result >> i) & 1);
                    parity = _parity;
                    break;
            }
            break;
        case ALUM:
            switch(alu_op(_instr)){
                case ADD:
                    _result = regs[REGS_A] + readByte(mem_addr());
                    regs[REGS_A] = _result;
                    carry = (_result > 0x00FF);
                    zero = (_result == 0);
                    sign = (_result & 0x80);
                    for(int i = 0; i < 8; i++) _parity ^= ((_result >> i) & 1);
                    parity = _parity;
                    break;
                case ADC:
                    _result = regs[REGS_A] + readByte(mem_addr()) + ((carry) ? 1 : 0);
                    regs[REGS_A] = _result;
                    carry = (_result > 0x00FF);
                    zero = (_result == 0);
                    sign = (_result & 0x80);
                    for(int i = 0; i < 8; i++) _parity ^= ((_result >> i) & 1);
                    parity = _parity;
                    break;
                case SUB:
                    _result = regs[REGS_A] - readByte(mem_addr());
                    regs[REGS_A] = _result;
                    carry = (_result < 0x0000);
                    zero = (_result == 0);
                    sign = (_result & 0x80);
                    for(int i = 0; i < 8; i++) _parity ^= ((_result >> i) & 1);
                    parity = _parity;
                    break;
                case SBC:
                    _result = regs[REGS_A] - readByte(mem_addr()) - ((carry) ? 1 : 0);
                    regs[REGS_A] = _result;
                    carry = (_result < 0x0000);
                    zero = (_result == 0);
                    sign = (_result & 0x80);
                    for(int i = 0; i < 8; i++) _parity ^= ((_result >> i) & 1);
                    parity = _parity;
                    break;
                case AND:
                    carry = false;
                    regs[REGS_A] &= readByte(mem_addr());
                    zero = (regs[REGS_A] == 0);
                    sign = (regs[REGS_A] & 0x80);
                    for(int i = 0; i < 8; i++) _parity ^= ((_result >> i) & 1);
                    parity = _parity;
                    break;
                case XOR:
                    carry = false;
                    regs[REGS_A] ^= readByte(mem_addr());
                    zero = (regs[REGS_A] == 0);
                    sign = (regs[REGS_A] & 0x80);
                    for(int i = 0; i < 8; i++) _parity ^= ((_result >> i) & 1);
                    parity = _parity;
                    break;
                case OR:
                    carry = false;
                    regs[REGS_A] |= readByte(mem_addr());
                    zero = (regs[REGS_A] == 0);
                    sign = (regs[REGS_A] & 0x80);
                    for(int i = 0; i < 8; i++) _parity ^= ((_result >> i) & 1);
                    parity = _parity;
                    break;
                case CP:
                    _result = regs[REGS_A] - readByte(mem_addr());
                    carry = (_result < 0x0000);
                    zero = (_result == 0);
                    sign = (_result & 0x80);
                    for(int i = 0; i < 8; i++) _parity ^= ((_result >> i) & 1);
                    parity = _parity;
                    break;
            }
            break;
        case ALUI:
            switch(alu_op(_instr)){
                case ADD:
                    _result = regs[REGS_A] + readByte(pc[pc_p]++);
                    regs[REGS_A] = _result;
                    carry = (_result > 0x00FF);
                    zero = (_result == 0);
                    sign = (_result & 0x80);
                    for(int i = 0; i < 8; i++) _parity ^= ((_result >> i) & 1);
                    parity = _parity;
                    break;
                case ADC:
                    _result = regs[REGS_A] + readByte(pc[pc_p]++) + ((carry) ? 1 : 0);
                    regs[REGS_A] = _result;
                    carry = (_result > 0x00FF);
                    zero = (_result == 0);
                    sign = (_result & 0x80);
                    for(int i = 0; i < 8; i++) _parity ^= ((_result >> i) & 1);
                    parity = _parity;
                    break;
                case SUB:
                    _result = regs[REGS_A] - readByte(pc[pc_p]++);
                    regs[REGS_A] = _result;
                    carry = (_result < 0x0000);
                    zero = (_result == 0);
                    sign = (_result & 0x80);
                    for(int i = 0; i < 8; i++) _parity ^= ((_result >> i) & 1);
                    parity = _parity;
                    break;
                case SBC:
                    _result = regs[REGS_A] - readByte(pc[pc_p]++) - ((carry) ? 1 : 0);
                    regs[REGS_A] = _result;
                    carry = (_result < 0x0000);
                    zero = (_result == 0);
                    sign = (_result & 0x80);
                    for(int i = 0; i < 8; i++) _parity ^= ((_result >> i) & 1);
                    parity = _parity;
                    break;
                case AND:
                    carry = false;
                    regs[REGS_A] &= readByte(pc[pc_p]++);
                    zero = (regs[REGS_A] == 0);
                    sign = (regs[REGS_A] & 0x80);
                    for(int i = 0; i < 8; i++) _parity ^= ((_result >> i) & 1);
                    parity = _parity;
                    break;
                case XOR:
                    carry = false;
                    regs[REGS_A] ^= readByte(pc[pc_p]++);
                    zero = (regs[REGS_A] == 0);
                    sign = (regs[REGS_A] & 0x80);
                    for(int i = 0; i < 8; i++) _parity ^= ((_result >> i) & 1);
                    parity = _parity;
                    break;
                case OR:
                    carry = false;
                    regs[REGS_A] |= readByte(pc[pc_p]++);
                    zero = (regs[REGS_A] == 0);
                    sign = (regs[REGS_A] & 0x80);
                    for(int i = 0; i < 8; i++) _parity ^= ((_result >> i) & 1);
                    parity = _parity;
                    break;
                case CP:
                    _result = regs[REGS_A] - readByte(pc[pc_p]++);
                    carry = (_result < 0x0000);
                    zero = (_result == 0);
                    sign = (_result & 0x80);
                    for(int i = 0; i < 8; i++) _parity ^= ((_result >> i) & 1);
                    parity = _parity;
                    break;
            }
            break;
        case RLC:
            carry = (regs[REGS_A] & 0x80);
            regs[REGS_A] = (regs[REGS_A] << 1) | (regs[REGS_A] >> 7);
            break;
        case RRC:
            carry = (regs[REGS_A] & 0x01);
            regs[REGS_A] = (regs[REGS_A] >> 1) | (regs[REGS_A] << 7);
            break;
        case RAL:
            _result = (regs[REGS_A] & 0x80);
            regs[REGS_A] = (regs[REGS_A] << 1) | ((carry) ? 0x01 : 0);
            carry = _result;
            break;
        case RAR:
            _result = (regs[REGS_A] & 0x01);
            regs[REGS_A] = (regs[REGS_A] >> 1) | ((carry) ? 0x80 : 0);
            carry = _result;
            break;
        case JMP:
            pc[pc_p] = readWord(pc[pc_p]);
            break;
        case JFc:
            switch(condition(_instr)){
                case COND_CARRY:
                    if(!carry){
                        pc[pc_p] = readWord(pc[pc_p]);
                        _cycles += 2;
                    }
                    else{
                        pc[pc_p] += 2;
                    }
                    break;
                case COND_ZERO:
                    if(!zero){
                        pc[pc_p] = readWord(pc[pc_p]);
                        _cycles += 2;
                    }
                    else{
                        pc[pc_p] += 2;
                    }
                    break;
                case COND_SIGN:
                    if(!sign){
                        pc[pc_p] = readWord(pc[pc_p]);
                        _cycles += 2;
                    }
                    else{
                        pc[pc_p] += 2;
                    }
                    break;
                case COND_PARITY:
                    if(!parity){
                        pc[pc_p] = readWord(pc[pc_p]);
                        _cycles += 2;
                    }
                    else{
                        pc[pc_p] += 2;
                    }
                    break;
            }
            break;
        case JTc:
            switch(condition(_instr)){
                case COND_CARRY:
                    if(carry){
                        pc[pc_p] = readWord(pc[pc_p]);
                        _cycles += 2;
                    }
                    else{
                        pc[pc_p] += 2;
                    }
                    break;
                case COND_ZERO:
                    if(zero){
                        pc[pc_p] = readWord(pc[pc_p]);
                        _cycles += 2;
                    }
                    else{
                        pc[pc_p] += 2;
                    }
                    break;
                case COND_SIGN:
                    if(sign){
                        pc[pc_p] = readWord(pc[pc_p]);
                        _cycles += 2;
                    }
                    else{
                        pc[pc_p] += 2;
                    }
                    break;
                case COND_PARITY:
                    if(parity){
                        pc[pc_p] = readWord(pc[pc_p]);
                        _cycles += 2;
                    }
                    else{
                        pc[pc_p] += 2;
                    }
                    break;
            }
            break;
        case CAL:
            pc[pc_p + 1] = readWord(pc[pc_p]);
            pc[pc_p] += 2;
            pc_p = (pc_p + 1) & 0x07;
            break;
        case CFc:
            switch(condition(_instr)){
                case COND_CARRY:
                    if(!carry){
                        pc[pc_p + 1] = readWord(pc[pc_p]);
                        pc[pc_p] += 2;
                        pc_p = (pc_p + 1) & 0x07;
                        _cycles += 2;
                    }
                    else{
                        pc[pc_p] += 2;
                    }
                    break;
                case COND_ZERO:
                    if(!zero){
                        pc[pc_p + 1] = readWord(pc[pc_p]);
                        pc[pc_p] += 2;
                        pc_p = (pc_p + 1) & 0x07;
                        _cycles += 2;
                    }
                    else{
                        pc[pc_p] += 2;
                    }
                    break;
                case COND_SIGN:
                    if(!sign){
                        pc[pc_p + 1] = readWord(pc[pc_p]);
                        pc[pc_p] += 2;
                        pc_p = (pc_p + 1) & 0x07;
                        _cycles += 2;
                    }
                    else{
                        pc[pc_p] += 2;
                    }
                    break;
                case COND_PARITY:
                    if(!parity){
                        pc[pc_p + 1] = readWord(pc[pc_p]);
                        pc[pc_p] += 2;
                        pc_p = (pc_p + 1) & 0x07;
                        _cycles += 2;
                    }
                    else{
                        pc[pc_p] += 2;
                    }
                    break;
            }
            break;
        case CTc:
            switch(condition(_instr)){
                case COND_CARRY:
                    if(carry){
                        pc[pc_p + 1] = readWord(pc[pc_p]);
                        pc[pc_p] += 2;
                        pc_p = (pc_p + 1) & 0x07;
                        _cycles += 2;
                    }
                    else{
                        pc[pc_p] += 2;
                    }
                    break;
                case COND_ZERO:
                    if(zero){
                        pc[pc_p + 1] = readWord(pc[pc_p]);
                        pc[pc_p] += 2;
                        pc_p = (pc_p + 1) & 0x07;
                        _cycles += 2;
                    }
                    else{
                        pc[pc_p] += 2;
                    }
                    break;
                case COND_SIGN:
                    if(sign){
                        pc[pc_p + 1] = readWord(pc[pc_p]);
                        pc[pc_p] += 2;
                        pc_p = (pc_p + 1) & 0x07;
                        _cycles += 2;
                    }
                    else{
                        pc[pc_p] += 2;
                    }
                    break;
                case COND_PARITY:
                    if(parity){
                        pc[pc_p + 1] = readWord(pc[pc_p]);
                        pc[pc_p] += 2;
                        pc_p = (pc_p + 1) & 0x07;
                        _cycles += 2;
                    }
                    else{
                        pc[pc_p] += 2;
                    }
                    break;
            }
            break;
        case RET:
            pc_p = (pc_p - 1) & 0x07;
            break;
        case RFc:
            switch(condition(_instr)){
                case COND_CARRY:
                    if(!carry){
                        pc_p = (pc_p - 1) & 0x07;
                        _cycles += 2;
                    }
                    break;
                case COND_ZERO:
                    if(!zero){
                        pc_p = (pc_p - 1) & 0x07;
                        _cycles += 2;
                    }
                    break;
                case COND_SIGN:
                    if(!sign){
                        pc_p = (pc_p - 1) & 0x07;
                        _cycles += 2;
                    }
                    break;
                case COND_PARITY:
                    if(!parity){
                        pc_p = (pc_p - 1) & 0x07;
                        _cycles += 2;
                    }
                    break;
            }
            break;
        case RTc:
            switch(condition(_instr)){
                case COND_CARRY:
                    if(carry){
                        pc_p = (pc_p - 1) & 0x07;
                        _cycles += 2;
                    }
                    break;
                case COND_ZERO:
                    if(zero){
                        pc_p = (pc_p - 1) & 0x07;
                        _cycles += 2;
                    }
                    break;
                case COND_SIGN:
                    if(sign){
                        pc_p = (pc_p - 1) & 0x07;
                        _cycles += 2;
                    }
                    break;
                case COND_PARITY:
                    if(parity){
                        pc_p = (pc_p - 1) & 0x07;
                        _cycles += 2;
                    }
                    break;
            }
            break;
        case RST:
            pc[++pc_p] = rst_addr(_instr);
            break;
        case INP:
            regs[REGS_A] = readDevice(device_num(_instr));
            break;
        case OUTP:
            writeDevice(device_num(_instr), regs[REGS_A]);
            break;
        case HLT:
            halt = true;
            break;
        default:
            break;
    }
    return _cycles;
}

void cpu8008::update_counters(uint32_t _count){
    cycle_counter += _count;
}

uint8_t cpu8008::readByte(uint16_t _address){
    last_read.push_back(_address);
    return mem[_address & 0x3FFF];
}

uint16_t cpu8008::readWord(uint16_t _address){
    return ((readByte(_address + 1) << 8) | readByte(_address));
}

void cpu8008::writeByte(uint16_t _address, uint8_t _data){
    last_write.push_back(_address);
    mem[_address & 0x3FFF] = _data;
}

uint8_t cpu8008::readDevice(uint8_t _deviceNum){
    return 0xFF;
}

void cpu8008::writeDevice(uint8_t _deviceNum, uint8_t _data){
    NULL;
}

string cpu8008::debug_instr_info(uint16_t _address){
    uint8_t _instr = readByte(_address);
    string _addr_string = "", _instr_byte_string = "", _instr_str = "";
    char _char_buf[10];
    
    sprintf(_char_buf, "0x%04X", _address);
    _addr_string = string(_char_buf) + string("  ");
    
    for(int i = 0; i < instruction_length[decode_instruction(_instr)]; i++){
        sprintf(_char_buf, "%02X", readByte(_address + i));
        _instr_byte_string += string(_char_buf) + string(" ");
    }
    for(int i = _instr_byte_string.size(); i < 10; i++) _instr_byte_string += " ";
    
    switch(decode_instruction(_instr)){
        case Lrr:
            _instr_str += "LD      ";
            switch(dst_reg(_instr)){
                case REGS_A: _instr_str += "A"; break;
                case REGS_B: _instr_str += "B"; break;
                case REGS_C: _instr_str += "C"; break;
                case REGS_D: _instr_str += "D"; break;
                case REGS_E: _instr_str += "E"; break;
                case REGS_H: _instr_str += "H"; break;
                case REGS_L: _instr_str += "L"; break;
            }
            _instr_str += ", ";
            switch(src_reg(_instr)){
                case REGS_A: _instr_str += "A"; break;
                case REGS_B: _instr_str += "B"; break;
                case REGS_C: _instr_str += "C"; break;
                case REGS_D: _instr_str += "D"; break;
                case REGS_E: _instr_str += "E"; break;
                case REGS_H: _instr_str += "H"; break;
                case REGS_L: _instr_str += "L"; break;
            }
            break;
        case LrM:
            _instr_str += "LD      ";
            switch(dst_reg(_instr)){
                case REGS_A: _instr_str += "A"; break;
                case REGS_B: _instr_str += "B"; break;
                case REGS_C: _instr_str += "C"; break;
                case REGS_D: _instr_str += "D"; break;
                case REGS_E: _instr_str += "E"; break;
                case REGS_H: _instr_str += "H"; break;
                case REGS_L: _instr_str += "L"; break;
            }
            _instr_str += ", ";
            sprintf(_char_buf, "0x%04X", mem_addr());
            _instr_str += string(_char_buf);
            break;
        case LMr:
            _instr_str += "LD      ";
            sprintf(_char_buf, "0x%04X", mem_addr());
            _instr_str += string(_char_buf) + string(", ");
            switch(src_reg(_instr)){
                case REGS_A: _instr_str += "A"; break;
                case REGS_B: _instr_str += "B"; break;
                case REGS_C: _instr_str += "C"; break;
                case REGS_D: _instr_str += "D"; break;
                case REGS_E: _instr_str += "E"; break;
                case REGS_H: _instr_str += "H"; break;
                case REGS_L: _instr_str += "L"; break;
            }
            break;
        case LrI:
            _instr_str += "LD      ";
            switch(dst_reg(_instr)){
                case REGS_A: _instr_str += "A"; break;
                case REGS_B: _instr_str += "B"; break;
                case REGS_C: _instr_str += "C"; break;
                case REGS_D: _instr_str += "D"; break;
                case REGS_E: _instr_str += "E"; break;
                case REGS_H: _instr_str += "H"; break;
                case REGS_L: _instr_str += "L"; break;
            }
            _instr_str += ", ";
            sprintf(_char_buf, "#0x%02X", readByte(_address + 1));
            _instr_str += string(_char_buf);
            break;
        case LMI:
            _instr_str += "LD      ";
            sprintf(_char_buf, "0x%04X", mem_addr());
            _instr_str += string(_char_buf) + string(", ");
            sprintf(_char_buf, "#0x%02X", readByte(_address + 1));
            _instr_str += string(_char_buf);
            break;
        case INr:
            _instr_str += "INC     ";
            switch(dst_reg(_instr)){
                case REGS_A: _instr_str += "A"; break;
                case REGS_B: _instr_str += "B"; break;
                case REGS_C: _instr_str += "C"; break;
                case REGS_D: _instr_str += "D"; break;
                case REGS_E: _instr_str += "E"; break;
                case REGS_H: _instr_str += "H"; break;
                case REGS_L: _instr_str += "L"; break;
            }
            break;
        case DCr:
            _instr_str += "DEC     ";
            switch(dst_reg(_instr)){
                case REGS_A: _instr_str += "A"; break;
                case REGS_B: _instr_str += "B"; break;
                case REGS_C: _instr_str += "C"; break;
                case REGS_D: _instr_str += "D"; break;
                case REGS_E: _instr_str += "E"; break;
                case REGS_H: _instr_str += "H"; break;
                case REGS_L: _instr_str += "L"; break;
            }
            break;
        case ALUr:
            switch(alu_op(_instr)){
                case ADD: _instr_str += "ADD     "; break;
                case ADC: _instr_str += "ADC     "; break;
                case SUB: _instr_str += "SUB     "; break;
                case SBC: _instr_str += "SBC     "; break;
                case AND: _instr_str += "AND     "; break;
                case XOR: _instr_str += "XOR     "; break;
                case OR:  _instr_str += "OR      "; break;
                case CP:  _instr_str += "CP      "; break;
            }
            switch(src_reg(_instr)){
                case REGS_A: _instr_str += "A"; break;
                case REGS_B: _instr_str += "B"; break;
                case REGS_C: _instr_str += "C"; break;
                case REGS_D: _instr_str += "D"; break;
                case REGS_E: _instr_str += "E"; break;
                case REGS_H: _instr_str += "H"; break;
                case REGS_L: _instr_str += "L"; break;
            }
            break;
        case ALUM:
            switch(alu_op(_instr)){
                case ADD: _instr_str += "ADD     "; break;
                case ADC: _instr_str += "ADC     "; break;
                case SUB: _instr_str += "SUB     "; break;
                case SBC: _instr_str += "SBC     "; break;
                case AND: _instr_str += "AND     "; break;
                case XOR: _instr_str += "XOR     "; break;
                case OR:  _instr_str += "OR      "; break;
                case CP:  _instr_str += "CP      "; break;
            }
            sprintf(_char_buf, "0x%04X", mem_addr());
            _instr_str += string(_char_buf);
            break;
        case ALUI:
            switch(alu_op(_instr)){
                case ADD: _instr_str += "ADD     "; break;
                case ADC: _instr_str += "ADC     "; break;
                case SUB: _instr_str += "SUB     "; break;
                case SBC: _instr_str += "SBC     "; break;
                case AND: _instr_str += "AND     "; break;
                case XOR: _instr_str += "XOR     "; break;
                case OR:  _instr_str += "OR      "; break;
                case CP:  _instr_str += "CP      "; break;
            }
            sprintf(_char_buf, "#0x%02X", readByte(_address + 1));
            _instr_str += string(_char_buf);
            break;
        case RLC:
            _instr_str += "RLC     ";
            break;
        case RRC:
            _instr_str += "RRC     ";
            break;
        case RAL:
            _instr_str += "RAL     ";
            break;
        case RAR:
            _instr_str += "RAR     ";
            break;
        case JMP:
            _instr_str += "JMP     ";
            sprintf(_char_buf, "0x%04X", readWord(_address + 1));
            _instr_str += string(_char_buf);
            break;
        case JFc:
            _instr_str += "JMP     ";
            switch(condition(_instr)){
                case COND_CARRY:  _instr_str += "NC"; break;
                case COND_ZERO:   _instr_str += "NZ"; break;
                case COND_SIGN:   _instr_str += "NS"; break;
                case COND_PARITY: _instr_str += "NP"; break;
            }
            sprintf(_char_buf, "0x%04X", readWord(_address + 1));
            _instr_str += string(", ") + string(_char_buf);
            break;
        case JTc:
            _instr_str += "JMP     ";
            switch(condition(_instr)){
                case COND_CARRY:  _instr_str += "C"; break;
                case COND_ZERO:   _instr_str += "Z"; break;
                case COND_SIGN:   _instr_str += "S"; break;
                case COND_PARITY: _instr_str += "P"; break;
            }
            sprintf(_char_buf, "0x%04X", readWord(_address + 1));
            _instr_str += string(", ") + string(_char_buf);
            break;
        case CAL:
            _instr_str += "CALL    ";
            sprintf(_char_buf, "0x%04X", readWord(_address + 1));
            _instr_str += string(_char_buf);
            break;
        case CFc:
            _instr_str += "CALL    ";
            switch(condition(_instr)){
                case COND_CARRY:  _instr_str += "NC"; break;
                case COND_ZERO:   _instr_str += "NZ"; break;
                case COND_SIGN:   _instr_str += "NS"; break;
                case COND_PARITY: _instr_str += "NP"; break;
            }
            sprintf(_char_buf, "0x%04X", readWord(_address + 1));
            _instr_str += string(", ") + string(_char_buf);
            break;
        case CTc:
            _instr_str += "CALL    ";
            switch(condition(_instr)){
                case COND_CARRY:  _instr_str += "C"; break;
                case COND_ZERO:   _instr_str += "Z"; break;
                case COND_SIGN:   _instr_str += "S"; break;
                case COND_PARITY: _instr_str += "P"; break;
            }
            sprintf(_char_buf, "0x%04X", readWord(_address + 1));
            _instr_str += string(", ") + string(_char_buf);
            break;
        case RET:
            _instr_str += "RET     ";
            break;
        case RFc:
            _instr_str += "RET     ";
            switch(condition(_instr)){
                case COND_CARRY:  _instr_str += "NC"; break;
                case COND_ZERO:   _instr_str += "NZ"; break;
                case COND_SIGN:   _instr_str += "NS"; break;
                case COND_PARITY: _instr_str += "NP"; break;
            }
            break;
        case RTc:
            _instr_str += "RET     ";
            switch(condition(_instr)){
                case COND_CARRY:  _instr_str += "C"; break;
                case COND_ZERO:   _instr_str += "Z"; break;
                case COND_SIGN:   _instr_str += "S"; break;
                case COND_PARITY: _instr_str += "P"; break;
            }
            break;
        case RST:
            _instr_str += "RST";
            break;
        case INP:
            _instr_str += "INP     ";
            sprintf(_char_buf, "0x%02X", device_num(_instr));
            _instr_str += string(_char_buf);
            break;
        case OUTP:
            _instr_str += "OUTP    ";
            sprintf(_char_buf, "0x%02X", device_num(_instr));
            _instr_str += string(_char_buf);
            break;
        case HLT:
            _instr_str += "HALT";
            break;
        default:
            _instr_str += "UNKNOWN";
            break;
    }
    return _addr_string + _instr_byte_string + _instr_str;
}

string cpu8008::debug_byte_info(uint16_t _address){
    string _addr_string = "", _instr_byte_string = "", _instr_str = "";
    char _char_buf[10];
    
    sprintf(_char_buf, "0x%04X", _address);
    _addr_string = string(_char_buf) + string("  ");
    
    sprintf(_char_buf, "%02X", readByte(_address));
    _instr_byte_string += string(_char_buf) + string(" ");
    
    for(int i = _instr_byte_string.size(); i < 10; i++) _instr_byte_string += " ";
    
    _instr_str = ".BYTE   0x" + string(_char_buf);
    
    return _addr_string + _instr_byte_string + _instr_str;
}

string cpu8008::debug_unknown_info(uint16_t _address){
    string _addr_string = "", _instr_byte_string = "", _instr_str = "";
    char _char_buf[10];
    
    sprintf(_char_buf, "0x%04X", _address);
    _addr_string = string(_char_buf) + string("  ");
    
    sprintf(_char_buf, "%02X", readByte(_address));
    _instr_byte_string += string(_char_buf) + string(" ");
    
    for(int i = _instr_byte_string.size(); i < 10; i++) _instr_byte_string += " ";
    
    _instr_str = "UNKNOWN 0x" + string(_char_buf);
    
    return _addr_string + _instr_byte_string + _instr_str;
}

string cpu8008::debug_cpu_info(){
    const char _debug_cpu_info_format_str[] =
            "PC: 0x%04X\n"
            "Stack: 0x%02X\n"
            "A: 0x%02X\tB: 0x%02X\n"
            "C: 0x%02X\tD: 0x%02X\n"
            "E: 0x%02X\tH: 0x%02X\n"
            "L: 0x%02X\tFlags: 0x%02X\n"
            "HL: 0x%04X\n";
    char _char_buf[127];
    sprintf(_char_buf, _debug_cpu_info_format_str,
            pc[pc_p], pc_p, regs[REGS_A], regs[REGS_B], regs[REGS_C],
            regs[REGS_D], regs[REGS_E], regs[REGS_H], regs[REGS_L], cpu_flags(), mem_addr());
    return _char_buf;
}

uint8_t cpu8008::get_instruction_length(uint16_t _address){
    return instruction_length[decode_instruction(readByte(_address))];
}

//switch(decode_instruction(_instr)){
//    case Lrr:
//    case LrM:
//    case LMr:
//    case LrI:
//    case LMI:
//    case INr:
//    case DCr:
//    case ALUr:
//    case ALUM:
//    case ALUI:
//    case RLC:
//    case RRC:
//    case RAL:
//    case RAR:
//    case JMP:
//    case JFc:
//    case JTc:
//    case CAL:
//    case CFc:
//    case CTc:
//    case RET:
//    case RFc:
//    case RTc:
//    case RST:
//    case INP:
//    case OUTP:
//    case HLT:
//    default:
//}
