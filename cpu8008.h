/* CPU8008 - Intel 8008 Emulator, with emulated videocontroller
 * By Yasin Morsli
*/
#ifndef CPU8008_H
#define CPU8008_H

using namespace std;

#include <iostream>
#include <vector>
#include <string>

#define elements(a)     (sizeof(a) / sizeof(a[0]))

class cpu8008{
    public:
        inline uint8_t src_reg(uint8_t _a)      {return (_a & 0x7);}
        inline uint8_t dst_reg(uint8_t _a)      {return ((_a >> 3) & 0x7);}
        inline uint8_t alu_op(uint8_t _a)       {return ((_a >> 3) & 0x7);}
        inline uint8_t condition(uint8_t _a)    {return ((_a >> 3) & 0x3);}
        inline uint8_t rst_addr(uint8_t _a)     {return (_a & 0x38);}
        inline uint8_t device_num(uint8_t _a)   {return ((_a >> 1) & 0x1F);}
        inline uint16_t mem_addr()              {return ((regs[REGS_H] << 8) | (regs[REGS_L]));}
        inline uint8_t cpu_flags()              {return (((carry) ? 8:0) | ((parity) ? 4:0) | ((zero) ? 2:0) | ((sign) ? 1:0));}
        
        enum instruction_t {
            Lrr = 0, LrM = 1, LMr = 2, LrI = 3, LMI = 4, INr = 5, DCr = 6,
            ALUr = 7, ALUM = 8, ALUI = 9, RLC = 10, RRC = 11, RAL = 12, RAR = 13,
            JMP = 14, JFc = 15, JTc = 16, CAL = 17, CFc = 18, CTc = 19, RET = 20, RFc = 21, RTc = 22, RST = 23,
            INP = 24, OUTP = 25, HLT = 26, UNKNOWN = 27
        };
        
        const uint8_t instruction_length[28] = {
            1, 1, 1, 2, 2, 1, 1,
            1, 1, 2, 1, 1, 1, 1,
            3, 3, 3, 3, 3, 3, 1, 1, 1, 1,
            1, 1, 1, 1
        };
        
        const uint8_t cycle_counts[28] = {
            5, 8, 7, 8, 9, 5, 5,
            5, 8, 8, 5, 5, 5, 5,
            11, 9, 9, 11, 9, 9, 5, 3, 3, 5,
            8, 6, 4, 5
        };
        
        enum alu_op_t {ADD = 0, ADC = 1, SUB = 2, SBC = 3, AND = 4, XOR = 5, OR = 6, CP = 7};
        enum condition_t {COND_CARRY = 0, COND_ZERO = 1, COND_SIGN = 2, COND_PARITY = 3};
        
        class Breakpoint{
            uint16_t address;
            bool enabled;
            
            Breakpoint(uint16_t _address, bool _enabled);
            bool is_hit();
        };
        
        uint32_t speed;
        uint32_t cycle_counter;
        vector<Breakpoint> breakpoints;
        vector<uint16_t> last_read;
        vector<uint16_t> last_write;
        instruction_t last_instruction;
        
        uint8_t mem[0x4000];
        uint16_t pc[8];
        uint8_t pc_p;
        uint8_t regs[7];
        enum regs_t {REGS_A = 0, REGS_B = 1, REGS_C = 2, REGS_D = 3, REGS_E = 4, REGS_H = 5, REGS_L = 6};
        bool carry, zero, sign, parity, halt;
        
        cpu8008();
        void reset();
        void step();
        void step_out();
        instruction_t decode_instruction(uint8_t _instr);
        uint8_t execute(uint8_t _instr);
        void update_counters(uint32_t _count);
        void addBreakpoint(Breakpoint _breakpoint);
        uint8_t readByte(uint16_t _address);
        uint16_t readWord(uint16_t _address);
        void writeByte(uint16_t _address, uint8_t _data);
        uint8_t readDevice(uint8_t _deviceNum);
        void writeDevice(uint8_t _deviceNum, uint8_t _data);
        string debug_instr_info(uint16_t _address);
        string debug_byte_info(uint16_t _address);
        string debug_unknown_info(uint16_t _address);
        string debug_cpu_info();
        uint8_t get_instruction_length(uint16_t _address);
        
};

#endif // CPU8008_H
