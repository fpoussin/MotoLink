/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#include "hal.h"

/*
 * Virtual serial ports over USB.
 */
SerialUSBDriver SDU1;
SerialUSBDriver SDU2;

#define USB_DEVICE_VID  0x0483
#define USB_DEVICE_PID  0xABCD
#define USB_DEVICE_GUID L"{656d69a0-4f42-45c4-b92c-bffa3b9e6bd1}\0"

/*
 * Endpoints.
 */
#define USB_INTERRUPT_REQUEST_EP_A      1
#define USB_DATA_AVAILABLE_EP_A         2
#define USB_DATA_REQUEST_EP_A           2
#define USB_INTERRUPT_REQUEST_EP_B      3
#define USB_DATA_AVAILABLE_EP_B         4
#define USB_DATA_REQUEST_EP_B           4

#define USB_INTERRUPT_REQUEST_SIZE      0x10
#define USB_DATA_SIZE                   0x40

/*
 * Interfaces
 */
#define USB_NUM_INTERFACES 4
#define USB_CDC_CIF_NUM0   0
#define USB_CDC_DIF_NUM0   1
#define USB_CDC_CIF_NUM1   2
#define USB_CDC_DIF_NUM1   3

/*
* Extra defines
*/

typedef struct
{
  uint8_t  bmRequestType;
  uint8_t  bRequest;
  uint16_t wValue;
  uint16_t wIndex;
  uint16_t wLength;
} USBSetupPkt;

/// Microsoft WCID descriptor
typedef struct {
    uint8_t bFirstInterfaceNumber;
    uint8_t reserved1;
    uint8_t compatibleID[8];
    uint8_t subCompatibleID[8];
    uint8_t reserved2[6];
} __attribute__((packed)) USBCompatIDDescIf;

typedef struct {
    uint32_t dwLength;
    uint16_t bcdVersion;
    uint16_t wIndex;
    uint8_t bCount;
    uint8_t reserved[7];
    USBCompatIDDescIf interfaces[];
} __attribute__((packed)) USBCompatIDDesc;

typedef struct {
    uint32_t	dwLength;
    uint16_t	bcdVersion;
    uint16_t	wIndex;
    uint16_t	bCount;
    uint32_t	dwPropertySize;
    uint32_t	dwPropertyDataType;
    uint16_t	wPropertyNameLength;
    wchar_t		PropertyName[21];
    uint32_t	dwPropertyDataLength;
    wchar_t		PropertyData[40];
} __attribute__((packed)) USBDExtPropertiesDesc;


#define	USB_REQ_OS_FEATURE  0x20
#define USB_FEATURE_PROPERTIES 0x04
#define USB_FEATURE_COMPAT_ID  0x05
#define USB_REQ_MS_DESCRIPTOR 0x04


/*
 * USB Device Descriptor.
 */
static const uint8_t usb_device_descriptor_data[] = {
  USB_DESC_DEVICE(
    0x0200,                                 /* bcdUSB (2.0).                */
    0xEF,                                   /* bDeviceClass (misc).         */
    0x02,                                   /* bDeviceSubClass (common).    */
    0x01,                                   /* bDeviceProtocol (IAD).       */
    USB_DATA_SIZE,                          /* bMaxPacketSize.              */
    USB_DEVICE_VID,                         /* idVendor.                    */
    USB_DEVICE_PID,                         /* idProduct.                   */
    0x0200,                                 /* bcdDevice.                   */
    1,                                      /* iManufacturer.               */
    2,                                      /* iProduct.                    */
    5,                                      /* iSerialNumber.               */
    1)                                      /* bNumConfigurations.          */
};

/*
 * Device Descriptor wrapper.
 */
static const USBDescriptor usb_device_descriptor = {
  sizeof usb_device_descriptor_data,
  usb_device_descriptor_data
};



#define BASE_IF_DESC_SET_SIZE ((USB_DESC_INTERFACE_SIZE*2) +  (USB_DESC_ENDPOINT_SIZE*3))

#define BASE_IF_DESC_SET(comIfNum, datIfNum, comInEp, datOutEp, datInEp)     \
  /* Interface Descriptor.*/                                                \
  USB_DESC_INTERFACE(                                                       \
    comIfNum,                               /* bInterfaceNumber.        */  \
    0x00,                                   /* bAlternateSetting.       */  \
    0x01,                                   /* bNumEndpoints.           */  \
    0xFF,                                   /* bInterfaceClass.         */  \
    0x00,                                   /* bInterfaceSubClass.      */  \
    0x00,                                   /* bInterfaceProtocol (AT
                                               commands, CDC section
                                               4.4).                    */  \
    0x04),                                     /* iInterface.              */  \
  /* Endpoint, Interrupt IN.*/                                              \
  USB_DESC_ENDPOINT (                                                       \
    comInEp,                                                                \
    USB_EP_MODE_TYPE_INTR,                  /* bmAttributes.            */  \
    USB_INTERRUPT_REQUEST_SIZE,             /* wMaxPacketSize.          */  \
    0x01),                                  /* bInterval.               */  \
                                                                            \
  /* CDC Data Interface Descriptor.*/                                       \
  USB_DESC_INTERFACE(                                                       \
    datIfNum,                               /* bInterfaceNumber.        */  \
    0x00,                                   /* bAlternateSetting.       */  \
    0x02,                                   /* bNumEndpoints.           */  \
    0xFF,                                   /* bInterfaceClass.         */  \
    0x00,                                   /* bInterfaceSubClass (CDC
                                               section 4.6).            */  \
    0x00,                                   /* bInterfaceProtocol (CDC
                                               section 4.7).            */  \
    0x04),                                  /* iInterface.              */  \
  /* Endpoint, Bulk OUT.*/                                                  \
  USB_DESC_ENDPOINT(                                                        \
    datOutEp,                               /* bEndpointAddress.        */  \
    USB_EP_MODE_TYPE_BULK,                  /* bmAttributes.            */  \
    USB_DATA_SIZE,                          /* wMaxPacketSize.          */  \
    0x00),                                  /* bInterval.               */  \
  /* Endpoint, Bulk IN.*/                                                   \
  USB_DESC_ENDPOINT(                                                        \
    datInEp,                                /* bEndpointAddress.        */  \
    USB_EP_MODE_TYPE_BULK,                  /* bmAttributes.            */  \
    USB_DATA_SIZE,                          /* wMaxPacketSize.          */  \
    0x00)                                   /* bInterval.               */

#define IAD_BASE_IF_DESC_SET_SIZE                                            \
  (USB_DESC_INTERFACE_ASSOCIATION_SIZE + BASE_IF_DESC_SET_SIZE)

#define IAD_BASE_IF_DESC_SET(comIfNum, datIfNum, comInEp, datOutEp, datInEp) \
  /* Interface Association Descriptor.*/                                    \
  USB_DESC_INTERFACE_ASSOCIATION(                                           \
    comIfNum,                               /* bFirstInterface.         */  \
    2,                                      /* bInterfaceCount.         */  \
    0xFF,                                   /* bFunctionClass.          */  \
    0x00,                                   /* bFunctionSubClass.       */  \
    0,                                      /* bFunctionProcotol.       */  \
    0x04                                    /* iInterface.              */  \
  ),                                                                        \
  /* CDC Interface descriptor set */                                        \
  BASE_IF_DESC_SET(comIfNum, datIfNum, comInEp, datOutEp, datInEp)




#define CDC_IF_DESC_SET_SIZE                                                \
  (USB_DESC_INTERFACE_SIZE + 5 + 5 + 4 + 5 + USB_DESC_ENDPOINT_SIZE +       \
   USB_DESC_INTERFACE_SIZE + (USB_DESC_ENDPOINT_SIZE * 2))

#define CDC_IF_DESC_SET(comIfNum, datIfNum, comInEp, datOutEp, datInEp)     \
  /* Interface Descriptor.*/                                                \
  USB_DESC_INTERFACE(                                                       \
    comIfNum,                               /* bInterfaceNumber.        */  \
    0x00,                                   /* bAlternateSetting.       */  \
    0x01,                                   /* bNumEndpoints.           */  \
    CDC_COMMUNICATION_INTERFACE_CLASS,      /* bInterfaceClass.         */  \
    CDC_ABSTRACT_CONTROL_MODEL,             /* bInterfaceSubClass.      */  \
    0x01,                                   /* bInterfaceProtocol (AT
                                               commands, CDC section
                                               4.4).                    */  \
    0x03),                                  /* iInterface.              */  \
  /* Header Functional Descriptor (CDC section 5.2.3).*/                    \
  USB_DESC_BYTE     (5),                    /* bLength.                 */  \
  USB_DESC_BYTE     (CDC_CS_INTERFACE),     /* bDescriptorType.         */  \
  USB_DESC_BYTE     (CDC_HEADER),           /* bDescriptorSubtype.      */  \
  USB_DESC_BCD      (0x0110),               /* bcdCDC.                  */  \
  /* Call Management Functional Descriptor.*/                               \
  USB_DESC_BYTE     (5),                    /* bFunctionLength.         */  \
  USB_DESC_BYTE     (CDC_CS_INTERFACE),     /* bDescriptorType.         */  \
  USB_DESC_BYTE     (CDC_CALL_MANAGEMENT),  /* bDescriptorSubtype.      */  \
  USB_DESC_BYTE     (0x03),    /*******/    /* bmCapabilities.          */  \
  USB_DESC_BYTE     (datIfNum),             /* bDataInterface.          */  \
  /* Abstract Control Management Functional Descriptor.*/                   \
  USB_DESC_BYTE     (4),                    /* bFunctionLength.         */  \
  USB_DESC_BYTE     (CDC_CS_INTERFACE),     /* bDescriptorType.         */  \
  USB_DESC_BYTE     (CDC_ABSTRACT_CONTROL_MANAGEMENT),                      \
  USB_DESC_BYTE     (0x02),                 /* bmCapabilities.          */  \
  /* Union Functional Descriptor.*/                                         \
  USB_DESC_BYTE     (5),                    /* bFunctionLength.         */  \
  USB_DESC_BYTE     (CDC_CS_INTERFACE),     /* bDescriptorType.         */  \
  USB_DESC_BYTE     (CDC_UNION),            /* bDescriptorSubtype.      */  \
  USB_DESC_BYTE     (comIfNum),             /* bMasterInterface.        */  \
  USB_DESC_BYTE     (datIfNum),             /* bSlaveInterface.         */  \
  /* Endpoint, Interrupt IN.*/                                              \
  USB_DESC_ENDPOINT (                                                       \
    comInEp,                                                                \
    USB_EP_MODE_TYPE_INTR,                  /* bmAttributes.            */  \
    USB_INTERRUPT_REQUEST_SIZE,             /* wMaxPacketSize.          */  \
    0x01),                                  /* bInterval.               */  \
                                                                            \
  /* CDC Data Interface Descriptor.*/                                       \
  USB_DESC_INTERFACE(                                                       \
    datIfNum,                               /* bInterfaceNumber.        */  \
    0x00,                                   /* bAlternateSetting.       */  \
    0x02,                                   /* bNumEndpoints.           */  \
    CDC_DATA_INTERFACE_CLASS,               /* bInterfaceClass.         */  \
    0x00,                                   /* bInterfaceSubClass (CDC
                                               section 4.6).            */  \
    0x00,                                   /* bInterfaceProtocol (CDC
                                               section 4.7).            */  \
    0x03),                                  /* iInterface.              */  \
  /* Endpoint, Bulk OUT.*/                                                  \
  USB_DESC_ENDPOINT(                                                        \
    datOutEp,                               /* bEndpointAddress.        */  \
    USB_EP_MODE_TYPE_BULK,                  /* bmAttributes.            */  \
    USB_DATA_SIZE,                          /* wMaxPacketSize.          */  \
    0x00),                                  /* bInterval.               */  \
  /* Endpoint, Bulk IN.*/                                                   \
  USB_DESC_ENDPOINT(                                                        \
    datInEp,                                /* bEndpointAddress.        */  \
    USB_EP_MODE_TYPE_BULK,                  /* bmAttributes.            */  \
    USB_DATA_SIZE,                          /* wMaxPacketSize.          */  \
    0x00)                                   /* bInterval.               */

#define IAD_CDC_IF_DESC_SET_SIZE                                            \
  (USB_DESC_INTERFACE_ASSOCIATION_SIZE + CDC_IF_DESC_SET_SIZE)

#define IAD_CDC_IF_DESC_SET(comIfNum, datIfNum, comInEp, datOutEp, datInEp) \
  /* Interface Association Descriptor.*/                                    \
  USB_DESC_INTERFACE_ASSOCIATION(                                           \
    comIfNum,                               /* bFirstInterface.         */  \
    2,                                      /* bInterfaceCount.         */  \
    CDC_COMMUNICATION_INTERFACE_CLASS,      /* bFunctionClass.          */  \
    CDC_ABSTRACT_CONTROL_MODEL,             /* bFunctionSubClass.       */  \
    1,                                      /* bFunctionProcotol.       */  \
    0x03                                    /* iInterface.              */  \
  ),                                                                        \
  /* CDC Interface descriptor set */                                        \
  CDC_IF_DESC_SET(comIfNum, datIfNum, comInEp, datOutEp, datInEp)



/* Configuration Descriptor tree for a CDC.*/
static const uint8_t usb_configuration_descriptor_data[] = {
  /* Configuration Descriptor.*/
  USB_DESC_CONFIGURATION(
    USB_DESC_CONFIGURATION_SIZE +
    (IAD_CDC_IF_DESC_SET_SIZE + IAD_BASE_IF_DESC_SET_SIZE),         /* wTotalLength.                */
    USB_NUM_INTERFACES,                     /* bNumInterfaces.              */
    0x01,                                   /* bConfigurationValue.         */
    0,                                      /* iConfiguration.              */
    0xC0,                                   /* bmAttributes (self powered). */
    50                                      /* bMaxPower (100mA).           */
  ),
  IAD_CDC_IF_DESC_SET(
    USB_CDC_CIF_NUM0,
    USB_CDC_DIF_NUM0,
    USB_ENDPOINT_IN(USB_INTERRUPT_REQUEST_EP_A),
    USB_ENDPOINT_OUT(USB_DATA_AVAILABLE_EP_A),
    USB_ENDPOINT_IN(USB_DATA_REQUEST_EP_A)
  ),
  IAD_BASE_IF_DESC_SET(
    USB_CDC_CIF_NUM1,
    USB_CDC_DIF_NUM1,
    USB_ENDPOINT_IN(USB_INTERRUPT_REQUEST_EP_B),
    USB_ENDPOINT_OUT(USB_DATA_AVAILABLE_EP_B),
    USB_ENDPOINT_IN(USB_DATA_REQUEST_EP_B)
  ),
};

//

static const USBCompatIDDesc usb_compat_id_descriptor_data = {
    .dwLength = sizeof(USBCompatIDDesc) +
                1*sizeof(USBCompatIDDescIf),
    .bcdVersion = 0x0100,
    .wIndex = 0x0004,
    .bCount = 1,
    .reserved = {0, 0, 0, 0, 0, 0, 0},
    .interfaces = {
        {
            .bFirstInterfaceNumber = 0x04,
            .reserved1 = 1,
            .compatibleID = "WINUSB\0\0",
            .subCompatibleID = {0, 0, 0, 0, 0, 0, 0, 0},
            .reserved2 = {0, 0, 0, 0, 0, 0},
        }
    }
};


static const USBDExtPropertiesDesc usb_guid_descriptor_data = {
    .dwLength = sizeof(USBDExtPropertiesDesc),
    .bcdVersion = 0x0100,
    .wIndex = 0x0005,
    .bCount = 2,

    .dwPropertySize = 136,
    .dwPropertyDataType = 7,
    .wPropertyNameLength = 21*2,
    .PropertyName = L"DeviceInterfaceGUIDs",
    .dwPropertyDataLength = 40*2,
    .PropertyData = USB_DEVICE_GUID,
};


//



static const uint8_t usb_winusb_descriptor_data[] =
{
  USB_DESC_BYTE(18),                         /* bLength */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING),      /* bDescriptorType */
  'M',0,'S',0,'F',0,'T',0,'1',0,'0',0,'0',0, /* qwSignature */
  0x22,                                      /* bMS_VendorCode */
  0                                          /* bPad */
};

static const USBDescriptor usb_winusb_descriptor = {
  sizeof usb_winusb_descriptor_data,  usb_winusb_descriptor_data
};

static const USBDescriptor usb_compat_id_descriptor = {
  sizeof usb_compat_id_descriptor_data,
  (uint8_t*)&usb_compat_id_descriptor_data
};

static const USBDescriptor usb_guid_descriptor = {
  sizeof usb_guid_descriptor_data,
  (uint8_t*)&usb_guid_descriptor_data
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
  USB_DESC_BYTE(18),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'M', 0, 'o', 0, 't', 0, 'o', 0, 'l', 0, 'i', 0, 'n', 0, 'k', 0
};


/*
 * Device Description string.
 */
static const uint8_t usb_string3[] = {
  USB_DESC_BYTE(52),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'M', 0, 'o', 0, 't', 0, 'o', 0, 'l', 0, 'i', 0, 'n', 0, 'k', 0,
  ' ', 0, 'S', 0, 'e', 0, 'r', 0, 'i', 0, 'a', 0, 'l', 0, ' ', 0,
  'i', 0, 'n', 0, 't', 0, 'e', 0, 'r', 0, 'f', 0, 'a', 0, 'c', 0,
  'e', 0
};

/*
 * Device Description string.
 */
static const uint8_t usb_string4[] = {
  USB_DESC_BYTE(46),                    /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  'M', 0, 'o', 0, 't', 0, 'o', 0, 'l', 0, 'i', 0, 'n', 0, 'k', 0,
  ' ', 0, 'U', 0, 'S', 0, 'B', 0, ' ', 0, 'I', 0, 'n', 0, 't', 0,
  'e', 0, 'r', 0, 'f', 0, 'a', 0, 'c', 0, 'e', 0
};

/*
 * Serial Number string.
 */
static const uint8_t usb_string5[] = {
  USB_DESC_BYTE(8),                     /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  '0' + CH_KERNEL_MAJOR, 0,
  '0' + CH_KERNEL_MINOR, 0,
  '0' + CH_KERNEL_PATCH, 0
};

/*
 * Strings wrappers array.
 */
static const USBDescriptor usb_strings[] = {
  {sizeof usb_string0, usb_string0},
  {sizeof usb_string1, usb_string1},
  {sizeof usb_string2, usb_string2},
  {sizeof usb_string3, usb_string3},
  {sizeof usb_string3, usb_string4},
  {sizeof usb_string3, usb_string5},
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
        else if (dindex == 0xee) /* WinUSB */
          return &usb_winusb_descriptor;
  }
  return NULL;
}

/**
 * @brief   IN EP1 state.
 */
static USBInEndpointState ep1instate;

/**
 * @brief   EP1 initialization structure (IN only).
 */
static const USBEndpointConfig ep1config = {
  USB_EP_MODE_TYPE_INTR,
  NULL,
  sduInterruptTransmitted,
  NULL,
  USB_INTERRUPT_REQUEST_SIZE,
  0x0000,
  &ep1instate,
  NULL,
  1,
  NULL
};

/**
 * @brief   IN EP2 state.
 */
static USBInEndpointState ep2instate;

/**
 * @brief   OUT EP2 state.
 */
static USBOutEndpointState ep2outstate;

/**
 * @brief   EP2 initialization structure (both IN and OUT).
 */
static const USBEndpointConfig ep2config = {
  USB_EP_MODE_TYPE_BULK,
  NULL,
  sduDataTransmitted,
  sduDataReceived,
  USB_DATA_SIZE,
  USB_DATA_SIZE,
  &ep2instate,
  &ep2outstate,
  1,
  NULL
};

/**
 * @brief   IN EP3 state.
 */
static USBInEndpointState ep3instate;

/**
 * @brief   EP3 initialization structure (IN only).
 */
static const USBEndpointConfig ep3config = {
  USB_EP_MODE_TYPE_INTR,
  NULL,
  sduInterruptTransmitted,
  NULL,
  USB_INTERRUPT_REQUEST_SIZE,
  0x0000,
  &ep3instate,
  NULL,
  1,
  NULL
};

/**
 * @brief   IN EP4 state.
 */
static USBInEndpointState ep4instate;

/**
 * @brief   OUT EP4 state.
 */
static USBOutEndpointState ep4outstate;

/**
 * @brief   EP4 initialization structure (both IN and OUT).
 */
static const USBEndpointConfig ep4config = {
  USB_EP_MODE_TYPE_BULK,
  NULL,
  sduDataTransmitted,
  sduDataReceived,
  USB_DATA_SIZE,
  USB_DATA_SIZE,
  &ep4instate,
  &ep4outstate,
  1,
  NULL
};

/*
 * Handles the USB driver global events.
 */
static void usb_event(USBDriver *usbp, usbevent_t event) {
  extern SerialUSBDriver SDU1;
  extern SerialUSBDriver SDU2;

  switch (event) {
  case USB_EVENT_RESET:
  case USB_EVENT_UNCONFIGURED:
  case USB_EVENT_ADDRESS:
    return;
  case USB_EVENT_CONFIGURED:
    chSysLockFromISR();

    if (usbp->state == USB_ACTIVE) {
      /* Enables the endpoints specified into the configuration.
         Note, this callback is invoked from an ISR so I-Class functions
         must be used.*/
      usbInitEndpointI(usbp, USB_INTERRUPT_REQUEST_EP_A, &ep1config);
      usbInitEndpointI(usbp, USB_DATA_REQUEST_EP_A, &ep2config);
      usbInitEndpointI(usbp, USB_INTERRUPT_REQUEST_EP_B, &ep3config);
      usbInitEndpointI(usbp, USB_DATA_REQUEST_EP_B, &ep4config);

      /* Resetting the state of the CDC subsystem.*/
      sduConfigureHookI(&SDU1);
      sduConfigureHookI(&SDU2);
    }
    else if (usbp->state == USB_SELECTED) {
      usbDisableEndpointsI(usbp);
    }

    chSysUnlockFromISR();
    return;
  case USB_EVENT_SUSPEND:
    chSysLockFromISR();

    /* Disconnection event on suspend.*/
    sduDisconnectI(&SDU1);
    sduDisconnectI(&SDU2);

    chSysUnlockFromISR();
    return;
  case USB_EVENT_WAKEUP:
    return;
  case USB_EVENT_STALLED:
    return;
  }
  return;
}

/*
 * Handling messages not implemented in the default handler nor in the
 * SerialUSB handler.
 */
static bool requests_hook(USBDriver *usbp) {

  USBSetupPkt *setup = (USBSetupPkt*)usbp->setup;

  if (((setup->bmRequestType & USB_RTYPE_RECIPIENT_MASK) == USB_RTYPE_RECIPIENT_INTERFACE) &&
      (setup->bRequest == USB_REQ_SET_INTERFACE)) {
    usbSetupTransfer(usbp, NULL, 0, NULL);
    return true;
  }

  if (setup->bmRequestType & (USB_RTYPE_DIR_MASK | USB_RTYPE_TYPE_VENDOR))
  {
      if (setup->bRequest == USB_REQ_MS_DESCRIPTOR)
      {
          if (setup->wIndex == USB_FEATURE_COMPAT_ID &&
              setup->bmRequestType == (USB_RTYPE_DIR_MASK | USB_RTYPE_TYPE_VENDOR | USB_RTYPE_RECIPIENT_DEVICE))
          {
              usbSetupTransfer(usbp,
                               (uint8_t*)&usb_compat_id_descriptor,
                               setup->wLength,
                               NULL);
              return true;
          }

          else if (setup->wIndex == USB_FEATURE_PROPERTIES &&
                  (setup->bmRequestType == (USB_RTYPE_DIR_MASK | USB_RTYPE_TYPE_VENDOR | USB_RTYPE_RECIPIENT_DEVICE) ||
                   setup->bmRequestType == (USB_RTYPE_DIR_MASK | USB_RTYPE_TYPE_VENDOR | USB_RTYPE_RECIPIENT_INTERFACE)))
          {
              usbSetupTransfer(usbp,
                               (uint8_t*)&usb_guid_descriptor,
                               setup->wLength,
                               NULL);
              return true;
          }
      }
  }
  return sduRequestsHook(usbp);
}

/*
 * Handles the USB driver global events.
 */
static void sof_handler(USBDriver *usbp) {

  (void)usbp;

  osalSysLockFromISR();
  sduSOFHookI(&SDU1);
  sduSOFHookI(&SDU2);
  osalSysUnlockFromISR();
}

/*
 * USB driver configuration.
 */
const USBConfig usbcfg = {
  usb_event,
  get_descriptor,
  requests_hook,
  sof_handler
};

/*
 * Serial over USB driver configuration 1.
 */
const SerialUSBConfig serusbcfg1 = {
  &USBD1,
  USB_DATA_REQUEST_EP_A,
  USB_DATA_AVAILABLE_EP_A,
  USB_INTERRUPT_REQUEST_EP_A
};

/*
 * Serial over USB driver configuration 2.
 */
const SerialUSBConfig serusbcfg2 = {
  &USBD1,
  USB_DATA_REQUEST_EP_B,
  USB_DATA_AVAILABLE_EP_B,
  USB_INTERRUPT_REQUEST_EP_B
};

/*
 * USB will pull input low when connected.
 */
bool usbConnected(void) {

  return palReadPad(PORT_USB_DETECT, PAD_USB_DETECT) == PAL_LOW;
}
