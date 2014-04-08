
#include "ch.h"
#include "hal.h"
#include "bulk_usb.h"

/*===========================================================================*/
/* USB related stuff.                                                        */
/*===========================================================================*/


/*
 * DP resistor control is not possible on the STM32F3-Discovery, using stubs
 * for the connection macros.
 */
#define usb_lld_connect_bus(usbp)
#define usb_lld_disconnect_bus(usbp)

/*
 * Endpoints to be used for USBD1.
 */
#define USBD1_DATA_REQUEST_EP           1
#define USBD1_DATA_AVAILABLE_EP         1
#define USBD1_INTERRUPT_REQUEST_EP      2
#define USBD2_DATA_REQUEST_EP           3
#define USBD2_DATA_AVAILABLE_EP         3


extern SerialUSBDriver SDU1;
extern BulkUSBDriver BDU1;

extern const USBConfig usbcfg;
extern const SerialUSBConfig serusbcfg;
extern const BulkUSBConfig bulkusbcfg;
