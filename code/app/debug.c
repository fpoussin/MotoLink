#include "ch.h"

#pragma GCC push_options
#pragma GCC optimize ("O0")

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
#define FLT_BUS_VECTOR 11

void HardFault_Handler(void);

void NMI_Handler(void) {

  HardFault_Handler();
}
void MemManage_Handler(void) {

  HardFault_Handler();
}
void BusFault_Handler(void) {

  HardFault_Handler();
}
void UsageFault_Handler(void) {

  HardFault_Handler();
}

void HardFault_Handler(void)
{
   register volatile void *stack_pointer asm("sp");
   volatile void **HARDFAULT_PSP;
   volatile uint32_t hfsr = SCB->HFSR;
   volatile uint32_t cfsr = SCB->CFSR;
   volatile uint8_t fault = 0;
   volatile uint8_t reason = 0;
   (void) fault;
   (void) reason;

   thread_t *tp = tp = chThdGetSelfX();

   // Hijack the process stack pointer to make backtrace work
   asm volatile ("mrs %0, psp" : "=r"(HARDFAULT_PSP) : :);
   stack_pointer = HARDFAULT_PSP;

   if (hfsr & SCB_HFSR_FORCED_Msk)
   {
       fault = FLT_HARD;
       asm volatile("BKPT #01");
   }

   if (hfsr & SCB_HFSR_VECTTBL_Msk)
   {
       fault = FLT_BUS_VECTOR;

       if(((cfsr >> 16) & (1 << 10U)) != 0)
       {
           reason = 15;
           asm volatile("BKPT #01");
       }
   }

   if((cfsr & SCB_CFSR_USGFAULTSR_Msk) != 0)
   {
       fault = FLT_USAGE;

       if(((cfsr >> 16) & (1 << 9)) != 0)
       {
           reason = FLT_ZERODIV;
           asm volatile("BKPT #01");
       }

       else if(((cfsr >> 16) & (1 << 8)) != 0)
       {
           reason = FLT_UNALIGNED;
           asm volatile("BKPT #01");
       }

       else if(((cfsr >> 16) & (1 << 3)) != 0)
       {
           reason = FLT_NOCP;
           asm volatile("BKPT #01");
       }

       else if(((cfsr >> 16) & (1 << 2)) != 0)
       {
           reason = FLT_INVPC;
           asm volatile("BKPT #01");
       }

       else if(((cfsr >> 16) & (1 << 1)) != 0)
       {
           reason = FLT_INVSTATE;
           asm volatile("BKPT #01");
       }

       else if(((cfsr >> 16) & (1 << 0)) != 0)
       {
           reason = FLT_UNDEFINSTR;
           asm volatile("BKPT #01");
       }

   }

   else if((cfsr & SCB_CFSR_BUSFAULTSR_Msk) != 0)
   {
       fault = FLT_BUS;
       asm volatile("BKPT #01");
   }

   else if((cfsr & SCB_CFSR_MEMFAULTSR_Msk) != 0)
   {
      fault = FLT_MEM;
      asm volatile("BKPT #01");
   }

   port_disable();

   asm volatile("BKPT #01");
   while(1);
}
#pragma GCC pop_options
