#pragma once

/** Memory location switch */
//#define BUILD_GOTHIC_1_08k
//#define BUILD_GOTHIC_2_6_fix



#define THISPTR_OFFSET(x) (((char *)this) + (x))

// -- call macro from GothicX (thx, Zerxes!)
#define XCALL(uAddr)                    \
        __asm { mov esp, ebp    }       \
        __asm { pop ebp                 }       \
        __asm { mov eax, uAddr  }       \
        __asm { jmp eax                 }

#define INST_NOP 0x90
#define REPLACE_OP(addr, op) {unsigned char* a = (unsigned char*)addr; *a = op;}
#define REPLACE_CALL(addr, op) {REPLACE_OP(addr, op); \
	REPLACE_OP(addr+1, op); \
	REPLACE_OP(addr+2, op); \
	REPLACE_OP(addr+3, op); \
	REPLACE_OP(addr+4, op); }

#define REPLACE_RANGE(start, end_incl, op) {for(int i=start; i<=end_incl;i++){REPLACE_OP(i, op);}}

#ifdef BUILD_GOTHIC_1_08k
#include "GothicMemoryLocations1_08k.h"
#endif

#ifdef BUILD_GOTHIC_2_6_fix
#include "GothicMemoryLocations2_6_fix.h"
#endif
