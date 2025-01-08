
#include <string.h>
#include <stdbool.h>
#include "rexlang_vm.h"

void rexlang_init(struct rexlang_vm *vm)
{
	vm->ip = REXLANG_MEMOFFS_PRGM;
	vm->sp = sizeof(vm->s);
	memset(vm->m, 0, sizeof(vm->m));
	memset(vm->s, 0, sizeof(vm->s));
	memset(vm->st, 0, sizeof(vm->st));
}

void rexlang_exec(struct rexlang_vm *vm)
{
	
}
