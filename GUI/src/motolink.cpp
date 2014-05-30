#include "motolink.h"

Motolink::Motolink(QUsb *usb, QObject *parent) :
    QObject(parent)
{
    mUsb = usb;

    mGuid = "656d69a0-4f42-45c4-b92c-bffa3b9e6bd1";
    mPid = 0x0483;
    mVid = 0xABCD;

    mUsb->setGuid(mGuid);
    mUsb->setDeviceIds(mPid, mVid);
    mUsb->setEndPoints(0x83, 0x03);
}

void Motolink::connect()
{
    mUsb->open();
}

void Motolink::disconnect()
{
    mUsb->close();
}
