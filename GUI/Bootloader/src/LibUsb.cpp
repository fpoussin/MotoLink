/*
 * Copyright (c) 2011 Darren Hague & Eric Brandt
 *               Modified to suport Linux and OSX by Mark Liversedge
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "LibUsb.h"

QUsb::QUsb(QObject *parent) :
    QObject(parent)
{
    // dynamic load of QUsb on Windows, it is statically linked in Linux
    // this is to avoid dll conflicts where the lib has already been installed
    //this->intf = NULL;
    //this->device = NULL;
    //usb_set_debug(0);
    this->readEndpoint = 0x01 | LIBUSB_ENDPOINT_IN;
    this->writeEndpoint = 0x01  | LIBUSB_ENDPOINT_OUT;
}

QUsb::~QUsb()
{
    this->close();
}

qint32 QUsb::open()
{
    int r; //for return values
    ssize_t cnt; //holding number of devices in list
    r = libusb_init(&ctx); //initialize the library for the session we just declared
    if(r < 0) {
        qDebug()<<"Init Error "<<r; //there was an error
        return 1;
    }
    libusb_set_debug(ctx, 3); //set verbosity level to 3, as suggested in the documentation

    cnt = libusb_get_device_list(ctx, &devs); //get the list of devices
    if(cnt < 0) {
        qDebug()<<"Get Device Error"; //there was an error
        return 1;
    }

    dev_handle = libusb_open_device_with_vid_pid(ctx, USB_ST_VID, USB_DEVICE_PID); //these are vendorID and productID I found for my usb device
    if(dev_handle == NULL)
        qDebug()<<"Cannot open device";
    else
        qDebug()<<"Device Opened";
    libusb_free_device_list(devs, 1); //free the list, unref the devices in it

    if(libusb_kernel_driver_active(dev_handle, 0) == 1) { //find out if kernel driver is attached
        qDebug()<<"Kernel Driver Active";
        if(libusb_detach_kernel_driver(dev_handle, 0) == 0) //detach it
            qDebug()<<"Kernel Driver Detached!";
    }
    r = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device (mine had jsut 1)
    if(r < 0) {
        qDebug()<<"Cannot Claim Interface";
        return 1;
    }
    qDebug()<<"Claimed Interface";

    return 0;
}

void QUsb::close()
{
    if (this->dev_handle) {
        // stop any further write attempts whilst we close down
        qDebug() << "Closing USB connection...";

        libusb_release_interface(dev_handle, 0); //release the claimed interface
        libusb_close(dev_handle); //close the device we opened
        libusb_exit(ctx); //needs to be called to end the
    }
}

qint32 QUsb::read(QByteArray *buf, quint32 bytes)
{
    qint32 rc, actual, actual_tmp;
    QElapsedTimer timer;

    // check it isn't closed already
    if (!this->dev_handle) return -1;

    if (bytes == 0)
        return 0;

    actual = 0;
    actual_tmp = 0;
    uchar *buffer = new uchar[bytes];

    timer.start();
    while (!timer.hasExpired(USB_TIMEOUT_MSEC) && bytes-actual > 0) {
        rc = libusb_bulk_transfer(dev_handle, (this->readEndpoint), buffer+actual, bytes-actual, &actual_tmp, USB_TIMEOUT_MSEC);
        actual += actual_tmp;
        if (rc != 0) break;
    }
    // we clear the buffer.
    buf->clear();
    QString data, s;

    if (1) {
        for (qint32 i = 0; i < actual; i++) {
            buf->append(buffer[i]);
            data.append(s.sprintf("%02X",(uchar)buf->at(i))+":");
        }
        data.remove(data.size()-1, 1); //remove last colon
        qDebug() << "Received: " << data;
    }
    delete buffer;
    if (rc != 0)
    {
        if (rc == -110)
            qDebug() << "Timeout";
        else
            qCritical() << "libusb_bulk_transfer Error reading: " << rc;
        return rc;
    }
    //delete buffer;
    return actual;
}

qint32 QUsb::write(QByteArray *buf, quint32 bytes)
{
    qint32 rc, actual, actual_tmp;
    QElapsedTimer timer;

    // check it isn't closed
    if (!this->dev_handle) return -1;

    QString cmd, s;
        for (int i=0; i<buf->size(); i++) {
            cmd.append(s.sprintf("%02X",(uchar)buf->at(i))+":");
        }
        cmd.remove(cmd.size()-1, 1); //remove last colon
        qDebug() << "Sending" << buf->size() << "bytes:" << cmd;

    actual = 0;
    actual_tmp = 0;

    timer.start();
    while (!timer.hasExpired(USB_TIMEOUT_MSEC) && bytes-actual > 0) {
        rc = libusb_bulk_transfer(dev_handle, (this->writeEndpoint), (uchar*)buf->constData(), bytes, &actual, USB_TIMEOUT_MSEC);
        actual += actual_tmp;
        if (rc != 0) break;
    }

    if (rc != 0)
    {
        if (rc == -110)
            qDebug() << "Timeout";
        else if (rc == -2)
            qCritical() << "EndPoint not found";
        else
            qCritical() << "usb_bulk_write Error: "<< rc;
    }

    return actual;
}
