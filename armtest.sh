#arm-none-eabi-gcc -O2 -mthumb -DNDEBUG -Wa,-adhln -g -c rexlang_vm.c > rexlang_vm.s
arm-none-eabi-gcc -Os -mthumb -DNDEBUG -Wa,-adhln -g -c rexlang_vm.c > rexlang_vm.s
