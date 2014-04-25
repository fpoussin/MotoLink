#include "usb_config.h"


/*
 * Serial over USB Driver structure.
 */

BulkUSBDriver BDU1;

/*
 * USB Device Descriptor.
 */
static const uint8_t usb_device_descriptor_data[18] = {
  USB_DESC_DEVICE       (0x0200,        /* bcdUSB (2.0).                    */
                         0x00,          /* bDeviceClass (DFU).              */
                         0x00,          /* bDeviceSubClass.                 */
                         0x00,          /* bDeviceProtocol.                 */
                         0x40,          /* bMaxPacketSize.                  */
                         0x0483,        /* idVendor (ST).                   */
                         0xFEDC,        /* idProduct.                       */
                         0x0200,        /* bcdDevice.                       */
                         1,             /* iManufacturer.                   */
                         5,             /* iProduct.                        */
                         3,             /* iSerialNumber.                   */
                         1)             /* bNumConfigurations.              */
};

/*
 * Device Descriptor wrapper.
 */
static const USBDescriptor usb_device_descriptor = {
  sizeof usb_device_descriptor_data,
  usb_device_descriptor_data
};

/* Configuration Descriptor tree for a CDC.*/
static const uint8_t usb_configuration_descriptor_data[32] = {
  /* Configuration Descriptor.*/
  USB_DESC_CONFIGURATION(32,            /* wTotalLength.                    */
                         0x01,          /* bNumInterfaces.                  */
                         0x01,          /* bConfigurationValue.             */
                         5,             /* iConfiguration.                  */
                         0xC0,          /* bmAttributes (self powered).     */
                         50),           /* bMaxPower (100mA).               */
  /* Interface Descriptor.*/
  USB_DESC_INTERFACE    (0x00,          /* bInterfaceNumber.                */
                         0x00,          /* bAlternateSetting.               */
                         0x02,          /* bNumEndpoints.                   */
                         0xFE,          /* bInterfaceClass (Vendor Specific). */
                         0x01,
                         0x01,
                         4),         /* iInterface.                      */
  /* Endpoint 3 Descriptor.*/
  USB_DESC_ENDPOINT     (USBD2_DATA_AVAILABLE_EP,       /* bEndpointAddress.*/
                         0x02,          /* bmAttributes (Bulk).             */
                         0x0040,        /* wMaxPacketSize.                  */
                         0x00),         /* bInterval.                       */
  /* Endpoint 3 Descriptor.*/
  USB_DESC_ENDPOINT     (USBD2_DATA_REQUEST_EP|0x80,    /* bEndpointAddress.*/
                         0x02,          /* bmAttributes (Bulk).             */
                         0x0040,        /* wMaxPacketSize.                  */
                         0x00)          /* bInterval.                       */
};

/*
 * Configuration Descriptor wrapper.
 */
static const USBDescriptor usb_configuration_descriptor = {
  sizeof usb_configuration_descriptor_data,
  usb_configuration_descriptor_data
};

/*
 * U.S. English language identifier.
 */
static const uint8_t usb_string0[] = {
  USB_DESC_BYTE(4),                     /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  USB_DESC_WORD(0x0409)                 /* wLANGID (U.S. English).          */
};

/*
 * Vendor string.
 */
static const uint8_t usb_string1[] = {
  USB_DESC_BYTE(38),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'S', 0, 'T', 0, 'M', 0, 'i', 0, 'c', 0, 'r', 0, 'o', 0, 'e', 0,
  'l', 0, 'e', 0, 'c', 0, 't', 0, 'r', 0, 'o', 0, 'n', 0, 'i', 0,
  'c', 0, 's', 0
};

/*
 * Device Description string.
 */
static const uint8_t usb_string2[] = {
  USB_DESC_BYTE(56),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'C', 0, 'h', 0, 'i', 0, 'b', 0, 'i', 0, 'O', 0, 'S', 0, '/', 0,
  'R', 0, 'T', 0, ' ', 0, 'V', 0, 'i', 0, 'r', 0, 't', 0, 'u', 0,
  'a', 0, 'l', 0, ' ', 0, 'C', 0, 'O', 0, 'M', 0, ' ', 0, 'P', 0,
  'o', 0, 'r', 0, 't', 0
};

/*
 * Serial Number string.
 */
static const uint8_t usb_string3[] = {
  USB_DESC_BYTE(8),                     /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  '0' + CH_KERNEL_MAJOR, 0,
  '0' + CH_KERNEL_MINOR, 0,
  '0' + CH_KERNEL_PATCH, 0
};

/*
 * Device Description string.
 */
static const uint8_t usb_string4[] = {
  USB_DESC_BYTE(52),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'C', 0, 'h', 0, 'i', 0, 'b', 0, 'i', 0, 'O', 0, 'S', 0, '/', 0,
  'R', 0, 'T', 0, ' ', 0, 'B', 0, 'u', 0, 'l', 0, 'k', 0, ' ', 0,
  'I', 0, 'n', 0, 't', 0, 'e', 0, 'r', 0, 'f', 0, 'a', 0, 'c', 0,
  'e', 0
};

/*
 * Device Description string.
 */
static const uint8_t usb_string5[] = {
  USB_DESC_BYTE(36),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'C', 0, 'h', 0, 'i', 0, 'b', 0, 'i', 0, 'O', 0, 'S', 0, '/', 0,
  'R', 0, 'T', 0, ' ', 0, 'D', 0, 'e', 0, 'v', 0, 'i', 0, 'c', 0,
  'e', 0
};

/*
 * Strings wrappers array.
 */
static const USBDescriptor usb_strings[] = {
  {sizeof usb_string0, usb_string0},
  {sizeof usb_string1, usb_string1},
  {sizeof usb_string2, usb_string2},
  {sizeof usb_string3, usb_string3},
  {sizeof usb_string4, usb_string4},
  {sizeof usb_string5, usb_string5}
};

/*
 * Handles the GET_DESCRIPTOR callback. All required descriptors must be
 * handled here.
 */
static const USBDescriptor *get_descriptor(USBDriver *usbp,
                                           uint8_t dtype,
                                           uint8_t dindex,
                                           uint16_t lang) {

  (void)usbp;
  (void)lang;
  switch (dtype) {
  case USB_DESCRIPTOR_DEVICE:
    return &usb_device_descriptor;
  case USB_DESCRIPTOR_CONFIGURATION:
    return &usb_configuration_descriptor;
  case USB_DESCRIPTOR_STRING:
    if (dindex < 6)
      return &usb_strings[dindex];
  }
  return NULL;
}

/**
 * @brief   IN EP1 state.
 */
static USBInEndpointState ep1instate;

/**
 * @brief   OUT EP1 state.
 */
static USBOutEndpointState ep1outstate;

/**
 * @brief   EP1 initialization structure (both IN and OUT).
 */
static const USBEndpointConfig ep1config = {
  USB_EP_MODE_TYPE_BULK,
  NULL,
  bduDataTransmitted,
  bduDataReceived,
  0x0040,
  0x0040,
  &ep1instate,
  &ep1outstate,
  1,
  NULL
};

/*
 * Handles the USB driver global events.
 */
static void usb_event(USBDriver *usbp, usbevent_t event) {

  switch (event) {
  case USB_EVENT_RESET:
    return;
  case USB_EVENT_ADDRESS:
    return;
  case USB_EVENT_CONFIGURED:
    chSysLockFromIsr();

    /* Bulk IF config */
    usbInitEndpointI(usbp, USBD2_DATA_REQUEST_EP, &ep1config);

    /* Resetting the state of the Bulk driver subsystem.*/
    bduConfigureHookI(&BDU1);

    chSysUnlockFromIsr();
    return;
  case USB_EVENT_SUSPEND:
    return;
  case USB_EVENT_WAKEUP:
    return;
  case USB_EVENT_STALLED:
    return;
  }
  return;
}


/*
 * USB driver configuration.
 */
const USBConfig usbcfg = {
  usb_event,
  get_descriptor,
  NULL,
  NULL
};


/*
 * Bulk USB driver configuration.
 */
const BulkUSBConfig bulkusbcfg = {
  &USBD1,
  USBD2_DATA_REQUEST_EP,
  USBD2_DATA_AVAILABLE_EP
};


/*
 * USB will pull input low when connected.
 */
bool_t usbDetect(void) {

  return palReadPad(USB_DETECT_PORT, USB_DETECT_PAD) == PAL_LOW;
}
