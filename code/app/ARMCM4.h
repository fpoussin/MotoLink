/* --------  Configuration of the Cortex-M4 Processor and Core Peripherals  ------- */
#define ARM_MATH_CM4
#define __CM4_REV                 0x0001    /*!< Core revision r0p1                              */
#define __MPU_PRESENT             1         /*!< MPU present or not                              */
#define __Vendor_SysTickConfig    0         /*!< Set to 1 if different SysTick Config is used    */
#define __FPU_PRESENT             1         /*!< FPU present or not                              */

#include <core_cm4.h>                       /* Processor and core peripherals                    */