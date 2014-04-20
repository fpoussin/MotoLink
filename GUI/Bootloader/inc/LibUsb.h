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

#ifndef gc_QUsb_h
#define gc_QUsb_h

#include <QString>
#include <QDebug>
#include <QByteArray>
#include <QtEndian>
#include <compat.h>

#include <libusb-1.0/libusb.h>


const quint16 USB_ST_VID = 0x0483;
const quint16 USB_DEVICE_PID = 0xFEDC;
const quint8 USB_CONFIGURATION = 1;   /* The sole configuration. */
const quint8 USB_PIPE_IN = 0x81;   /* Bulk output endpoint for responses */
const quint8 USB_PIPE_OUT = 0x01;	   /* Bulk input endpoint for commands */
const quint16 USB_TIMEOUT_MSEC = 250;

class QUsb : public QObject {

public:
    explicit QUsb(QObject *parent = 0);
    ~QUsb();

public slots:
    qint32 open();
    void close();
    qint32 read(QByteArray *buf, quint32 bytes);
    qint32 write(QByteArray *buf, quint32 bytes);

private:
    libusb_device **devs;
    libusb_device_handle *dev_handle;
    libusb_context *ctx;

    quint8 readEndpoint, writeEndpoint;
    quint8 interface;
    quint8 alternate;
};
#endif
