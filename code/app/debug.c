#include "ch.h"

#define FLT_HARD  1
#define FLT_USAGE 2
#define FLT_ZERODIV 3
#define FLT_UNALIGNED 4
#define FLT_BUS 5
#define FLT_MEM 6
#define FLT_UNDEFINSTR 7
#define FLT_INVSTATE 8
#define FLT_INVPC 9
#define FLT_NOCP 10

void **HARDFAULT_PSP;
register void *stack_pointer asm("sp");
static volatile uint8_t fault = 0;
static volatile uint8_t reason = 0;

void HardFault_Handler(void)
{
   volatile uint32_t hfsr = SCB->HFSR;
   volatile uint32_t cfsr = SCB->CFSR;

   // Hijack the process stack pointer to make backtrace work
   //asm("mrs %0, psp" : "=r"(HARDFAULT_PSP) : :);
   //stack_pointer = HARDFAULT_PSP;

   if (hfsr & SCB_HFSR_FORCED_Msk)
   {
       fault = FLT_HARD;
   }

   if((cfsr & SCB_CFSR_USGFAULTSR_Msk) != 0)
   {
       fault = FLT_USAGE;

       if(((cfsr >> 16) & (1 << 9)) != 0)
       {
           reason = FLT_ZERODIV;
       }

       else if(((cfsr >> 16) & (1 << 8)) != 0)
       {
           reason = FLT_UNALIGNED;
       }

       else if(((cfsr >> 16) & (1 << 3)) != 0)
       {
           reason = FLT_NOCP;
       }

       else if(((cfsr >> 16) & (1 << 2)) != 0)
       {
           reason = FLT_INVPC;
       }

       else if(((cfsr >> 16) & (1 << 1)) != 0)
       {
           reason = FLT_INVSTATE;
       }

       else if(((cfsr >> 16) & (1 << 0)) != 0)
       {
           reason = FLT_UNDEFINSTR;
       }

   }

   else if((cfsr & SCB_CFSR_BUSFAULTSR_Msk) != 0)
   {
       fault = FLT_BUS;
   }

   else if((cfsr & SCB_CFSR_MEMFAULTSR_Msk) != 0)
   {
      fault = FLT_MEM;
   }

   __ASM volatile("BKPT #01");
   while(1);
}

