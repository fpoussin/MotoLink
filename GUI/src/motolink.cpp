#include "motolink.h"

Motolink::Motolink(QUsb *usb, QObject *parent) :
    QObject(parent)
{
    mUsb = usb;

    mGuid = "f0207e85-88d7-4839-bba8-bf87a5092f03";
    mPid = 0x0483;
    mVid = 0xFEDC;

    mUsb->setGuid(mGuid);
    mUsb->setDeviceIds(mPid, mVid);
}
