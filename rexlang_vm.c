
#include <string.h>
#include <stdbool.h>
#include "rexlang_vm.h"

void rex_vm_init(struct rexlang_vm *vm)
{
	memset(vm->m, 0, sizeof(vm->m));
}

void rex_vm_exec(struct rexlang_vm *vm)
{
	
}
