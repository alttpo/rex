
#ifndef _REX_H_
#define _REX_H_

#include <stdint.h>
#include "rexlang_vm.h"

#define REXLANG_VM_COUNT 2

struct rex_state {
	int cycles;
	int next_exec_cycle;
	struct rexlang_vm vm[REXLANG_VM_COUNT];
};

extern struct rex_state rex;

static void rex_advance_clock(int cycles) {
	rexlang_exec(&rex.vm[i]);
}

#endif
