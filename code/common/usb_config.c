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
#include "common.h"

/*
 * Virtual serial ports over USB.
 */
SerialUSBDriver SDU1;
SerialUSBDriver SDU2;
extern SerialConfig uart3Cfg;

#define USB_DEVICE_VID  0x0483
#define USB_DEVICE_PID  0xABCD
#define USB_DEVICE_GUID u"{1EE57D96-67C5-4E84-9CB7-DEEC7929B8A3}\0"

/*
 * Endpoints.
 */
#define USB_INTERRUPT_EP_A  1
#define USB_DATA_EP_A       2
#define USB_INTERRUPT_EP_B  3
#define USB_DATA_EP_B       4

#define USB_INTERRUPT_REQUEST_SIZE 0x10
#define USB_DATA_SIZE              0x40

/*
 * Interfaces
 */
#define USB_NUM_INTERFACES 4
#define USB_CDC_CIF_NUM0   0
#define USB_CDC_DIF_NUM0   1
#define USB_BASE_CIF_NUM1  2
#define USB_BASE_DIF_NUM1  3

/*
 * Current Line Coding.
 */
static cdc_linecoding_t linecoding = {
  {0x00, 0x96, 0x00, 0x00},             /* 38400.                           */
  LC_STOP_1, LC_PARITY_NONE, 8
};


/*
* Extra defines
*/

typedef struct  {
    const uint8_t bLength;
    const uint8_t bDescriptorType;
    const uint16_t bString[30]; // Unicode String
} __attribute__((packed)) USBStringDesc;

typedef struct
{
  uint8_t  bmRequestType;
  uint8_t  bRequest;
  uint16_t wValue;
  uint16_t wIndex;
  uint16_t wLength;
} __attribute__((packed)) USBSetupPkt;

/// Microsoft WCID descriptor
typedef struct {
    uint8_t bFirstInterfaceNumber;
    uint8_t bInterfaceCount;
    uint8_t compatibleID[8];
    uint8_t subCompatibleID[8];
    uint8_t reserved[6];
} __attribute__((packed)) USBCompatIDFunctionDesc;

typedef struct {
    uint32_t dwLength;
    uint16_t bcdVersion;
    uint16_t wIndex;
    uint8_t bCount;
    uint8_t reserved[7];
} __attribute__((packed)) USBCompatIDHeaderDesc;

typedef struct {
    USBCompatIDHeaderDesc header;
    USBCompatIDFunctionDesc sections[];
} __attribute__((packed)) USBCompatIDDesc;

typedef struct {
    uint32_t dwPropertySize;
    uint32_t dwPropertyDataType;
    uint16_t wPropertyNameLength;
    uint16_t PropertyName[40];
    uint32_t dwPropertyDataLength;
    uint16_t PropertyData[100];
} __attribute__((packed)) USBDExtPropertyDesc;

typedef struct {
    uint32_t dwLength;
    uint16_t bcdVersion;
    uint16_t wIndex;
    uint16_t bCount;
    USBDExtPropertyDesc properties[];
} __attribute__((packed)) USBDExtPropertiesDesc;


typedef struct {
  uint8_t  bLength;
  uint8_t  bDescriptorType;
  uint16_t bcdUSB;
  uint8_t  bDeviceClass;
  uint8_t  bDeviceSubClass;
  uint8_t  bDeviceProtocol;
  uint8_t  bMaxPacketSize0;
  uint8_t  bNumConfigurations;
  uint8_t  bReserved;
} __attribute__((packed)) USB_DEVICE_QUALIFIER_DESCRIPTOR;


#define	USB_REQ_OS_FEATURE  0x20
#define USB_FEATURE_PROPERTIES 0x05
#define USB_FEATURE_COMPAT_ID  0x04
#define USB_VENDOR_CODE 0x04


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

#define BASE_IF_DESC_SET(comIfNum, datIfNum, comInEp, datOutEp, datInEp)    \
  /* Interface Descriptor.*/                                                \
  USB_DESC_INTERFACE(                                                       \
    comIfNum,                               /* bInterfaceNumber.        */  \
    0x00,                                   /* bAlternateSetting.       */  \
    0x01,                                   /* bNumEndpoints.           */  \
    0xFF,                                   /* bInterfaceClass.         */  \
    0x00,                                   /* bInterfaceSubClass.      */  \
    0x00,                                   /* bInterfaceProtocol       */  \
    0x04),                                     /* iInterface.           */  \
  /* Endpoint, Interrupt IN.*/                                              \
  USB_DESC_ENDPOINT (                                                       \
    comInEp,                                                                \
    USB_EP_MODE_TYPE_INTR,                  /* bmAttributes.            */  \
    USB_INTERRUPT_REQUEST_SIZE,             /* wMaxPacketSize.          */  \
    0x01),                                  /* bInterval.               */  \
                                                                            \
  /* Base Data Interface Descriptor.*/                                      \
  USB_DESC_INTERFACE(                                                       \
    datIfNum,                               /* bInterfaceNumber.        */  \
    0x00,                                   /* bAlternateSetting.       */  \
    0x02,                                   /* bNumEndpoints.           */  \
    0xFF,                                   /* bInterfaceClass.         */  \
    0x00,                                   /* bInterfaceSubClass       */  \
    0x00,                                   /* bInterfaceProtocol       */  \
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

#define IAD_BASE_IF_DESC_SET_SIZE                                           \
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
  /* Base Interface descriptor set */                                       \
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
    USB_ENDPOINT_IN(USB_INTERRUPT_EP_A),
    USB_ENDPOINT_OUT(USB_DATA_EP_A),
    USB_ENDPOINT_IN(USB_DATA_EP_A)
  ),
  IAD_BASE_IF_DESC_SET(
    USB_BASE_CIF_NUM1,
    USB_BASE_DIF_NUM1,
    USB_ENDPOINT_IN(USB_INTERRUPT_EP_B),
    USB_ENDPOINT_OUT(USB_DATA_EP_B),
    USB_ENDPOINT_IN(USB_DATA_EP_B)
  ),
};

static const USBCompatIDDesc usb_compat_id_descriptor_data = {
    .header = {
        .dwLength = sizeof(USBCompatIDHeaderDesc) +
                    (1 * sizeof(USBCompatIDFunctionDesc)),
        .bcdVersion = 0x0100,
        .wIndex = 0x0004,
        .bCount = 1,
        .reserved = {0, 0, 0, 0, 0, 0, 0},
    },
    .sections = {
        {
            .bFirstInterfaceNumber = 0x02,
            .bInterfaceCount = 1,
            .compatibleID = "WINUSB\0\0",
            .subCompatibleID = {0, 0, 0, 0, 0, 0, 0, 0},
            .reserved = {0, 0, 0, 0, 0, 0},
        },
        {
            .bFirstInterfaceNumber = 0x03,
            .bInterfaceCount = 1,
            .compatibleID = "WINUSB\0\0",
            .subCompatibleID = {0, 0, 0, 0, 0, 0, 0, 0},
            .reserved = {0, 0, 0, 0, 0, 0},
        }
    }
};

static const USBDExtPropertiesDesc usb_guid_descriptor_data = {
    .dwLength = sizeof(USBDExtPropertiesDesc) +
            1 * sizeof(USBDExtPropertyDesc),
    .bcdVersion = 0x0100,
    .wIndex = 0x0005,
    .bCount = 1,
    .properties = {
        {
            .dwPropertySize = sizeof(USBDExtPropertyDesc),
            .dwPropertyDataType = 7,
            .wPropertyNameLength = 80,
            .PropertyName = u"DeviceInterfaceGUIDs\0\0",
            .dwPropertyDataLength = 200,
            .PropertyData = USB_DEVICE_GUID
        }
    }
};

static const uint8_t usb_winusb_descriptor_data[] =
{
  USB_DESC_BYTE(18),                         /* bLength */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING),      /* bDescriptorType */
  'M',0,'S',0,'F',0,'T',0,'1',0,'0',0,'0',0, /* qwSignature */
  USB_VENDOR_CODE,                     /* bMS_VendorCode */
  0                                          /* bPad */
};

static const USBDescriptor usb_winusb_descriptor = {
  sizeof usb_winusb_descriptor_data,  usb_winusb_descriptor_data
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

static const USBStringDesc usb_string0 = {
  USB_DESC_BYTE(4),                      /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING),  /* bDescriptorType.                 */
  {0x0409}                               /* wLANGID (U.S. English).          */
};

/*
 * Vendor string.
 */
static const USBStringDesc usb_string1 = {
  USB_DESC_BYTE(sizeof(USBStringDesc)),  /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING),  /* bDescriptorType.                 */
  u"STMicroelectronics"
};

/*
 * Device Description string.
 */
static const USBStringDesc usb_string2 = {
  USB_DESC_BYTE(sizeof(USBStringDesc)),  /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING),  /* bDescriptorType.                 */
  u"Motolink"
};


/*
 * Device Description string.
 */
static const USBStringDesc usb_string3 = {
  USB_DESC_BYTE(sizeof(USBStringDesc)),  /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING),  /* bDescriptorType.                 */
  u"Motolink Serial Interface"
};

/*
 * Device Description string.
 */
static const USBStringDesc usb_string4 = {
  USB_DESC_BYTE(sizeof(USBStringDesc)),  /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING),  /* bDescriptorType.                 */
  u"Motolink WINUSB Interface"
};

/*
 * Serial Number string.
 */
static const USBStringDesc usb_string5 = {
  USB_DESC_BYTE(8),                     /* bLength.                         */
  USB_DESC_BYTE(USB_DESCRIPTOR_STRING), /* bDescriptorType.                 */
  {u'0' + 1, u'0', u'0'}
};

/*
 * Strings wrappers array.
 */
static const USBDescriptor usb_strings[] = {
  {4, (uint8_t*)&usb_string0},
  {sizeof usb_string1, (uint8_t*)&usb_string1},
  {sizeof usb_string2, (uint8_t*)&usb_string2},
  {sizeof usb_string3, (uint8_t*)&usb_string3},
  {sizeof usb_string3, (uint8_t*)&usb_string4},
  {sizeof usb_string3, (uint8_t*)&usb_string5},
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
        return NULL;
      case USB_DESCRIPTOR_DEVICE_QUALIFIER: /* High speed - not needed  */
        return NULL;
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
      usbInitEndpointI(usbp, USB_INTERRUPT_EP_A, &ep1config);
      usbInitEndpointI(usbp, USB_DATA_EP_A, &ep2config);
      usbInitEndpointI(usbp, USB_INTERRUPT_EP_B, &ep3config);
      usbInitEndpointI(usbp, USB_DATA_EP_B, &ep4config);

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
    sduSuspendHookI(&SDU1);
    sduSuspendHookI(&SDU2);

    chSysUnlockFromISR();
    return;
  case USB_EVENT_WAKEUP:
    return;
  case USB_EVENT_STALLED:
    return;
  }
  return;
}

bool sduCustomRequestsHook(USBDriver *usbp) {

  USBSetupPkt *setup = (USBSetupPkt*)usbp->setup;

  if ((setup->bmRequestType & USB_RTYPE_TYPE_MASK) == USB_RTYPE_TYPE_CLASS) {
    switch (setup->bRequest) {
    case CDC_GET_LINE_CODING:
      usbSetupTransfer(usbp, (uint8_t *)&linecoding, sizeof(linecoding), NULL);
      return true;
    case CDC_SET_LINE_CODING:
      usbSetupTransfer(usbp, (uint8_t *)&linecoding, sizeof(linecoding), NULL);
      setLineCoding(&linecoding, &SD3, &uart3Cfg);
      return true;
    case CDC_SET_CONTROL_LINE_STATE:
      /* Nothing to do, there are no control lines.*/
      usbSetupTransfer(usbp, NULL, 0, NULL);
      return true;
    default:
      return false;
    }
  }
  return false;
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
      if (setup->bRequest == USB_VENDOR_CODE)
      {
          if (setup->wIndex == USB_FEATURE_COMPAT_ID)
          {
              usbSetupTransfer(usbp,
                               (uint8_t*)&usb_compat_id_descriptor_data,
                               setup->wLength,
                               NULL);
              return true;
          }

          if (setup->wIndex == USB_FEATURE_PROPERTIES)
          {
              usbSetupTransfer(usbp,
                               (uint8_t*)&usb_guid_descriptor_data,
                               setup->wLength,
                               NULL);
              return true;
          }
      }
  }
  return sduCustomRequestsHook(usbp);
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
  USB_DATA_EP_A,
  USB_DATA_EP_A,
  USB_INTERRUPT_EP_A
};

/*
 * Serial over USB driver configuration 2.
 */
const SerialUSBConfig serusbcfg2 = {
  &USBD1,
  USB_DATA_EP_B,
  USB_DATA_EP_B,
  USB_INTERRUPT_EP_B
};

/*
 * USB will pull input low when connected.
 */
bool usbConnected(void) {

  return palReadPad(PORT_USB_DETECT, PAD_USB_DETECT) == PAL_LOW;
}
