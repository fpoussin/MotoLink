#include "drivers.h"

void driversInit(void)
{
#if defined(HAL_USE_DAC) && HAL_USE_DAC
  dacInit();
#endif
#if defined(HAL_USE_IWDG) && HAL_USE_IWDG
  iwdgInit();
#endif
#if defined(HAL_USE_TIMCAP) && HAL_USE_TIMCAP
  timcapInit();
#endif
}
