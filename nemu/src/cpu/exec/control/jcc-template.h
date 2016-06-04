#include "cpu/exec/template-start.h"

#if DATA_BYTE == 1
#define CODE_LEN 2
#endif
#if DATA_BYTE == 2
#define CODE_LEN 4
#endif
#if DATA_BYTE == 4
#define CODE_LEN 6
#endif

#define get_new_eip()\
		int32_t val = op_src->val;\
		val = val << (32 - DATA_BYTE * 8);\
		val = val >> (32 - DATA_BYTE * 8);\
		uint32_t new_eip = cpu.eip + val;\
		if(DATA_BYTE == 2) new_eip &= 0xffff;\
		print_asm(str(instr) " $0x%x", new_eip + CODE_LEN);

#define instr je

static void do_execute() {
		get_new_eip();
		if(cpu.ZF == 1) cpu.eip = new_eip;
}

make_instr_helper(i)

#undef instr

#define instr jbe

static void do_execute() {
		get_new_eip();
		if(cpu.CF == 1 || cpu.ZF == 1) cpu.eip = new_eip;
}

make_instr_helper(i)

#undef instr

#undef CODE_LEN
#include "cpu/exec/template-end.h"
