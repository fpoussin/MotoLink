
#include "common.h"

/*===========================================================================*/
/* USB related stuff.                                                        */
/*===========================================================================*/

extern const USBConfig usbcfg;
extern SerialUSBConfig serusbcfg1;
extern SerialUSBConfig serusbcfg2;
extern SerialUSBDriver SDU1;
extern SerialUSBDriver SDU2;

extern bool doKLineInit;
extern SerialConfig uart3Cfg;

bool usbConnected(void);
bool usb_lld_connect_bus(USBDriver *usbp);
bool usb_lld_disconnect_bus(USBDriver *usbp);
