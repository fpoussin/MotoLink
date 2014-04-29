#include "motolink.h"

Motolink::Motolink(QUsb *usb, QObject *parent) :
    QObject(parent)
{
    mUsb = usb;
}
