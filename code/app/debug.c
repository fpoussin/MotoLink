#include "ch.h"

void **HARDFAULT_PSP;
register void *stack_pointer asm("sp");

void HardFaultVector(void)
{
    // Hijack the process stack pointer to make backtrace work
    asm("mrs %0, psp" : "=r"(HARDFAULT_PSP) : :);
    stack_pointer = HARDFAULT_PSP;
    while(1);
}
