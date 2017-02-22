#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <QObject>
#include <QString>
#include <QThread>
#include <QTimer>
#include <QUsb>
#include <QtEndian>
#include <compat.h>

typedef quint8 uint8_t;
typedef quint16 uint16_t;
typedef quint32 uint32_t;
#include "protocol.h"

//#define WAIT_USB _QThread::usleep(40000);
#define _WAIT_USB_

class Bootloader : public QObject {
  Q_OBJECT
public:
  explicit Bootloader(QUsbDevice *usb, QObject *parent = 0);
  ~Bootloader();

public slots:
  quint8 getFlags();
  bool boot();
  qint32 writeFlash(quint32 addr, const QByteArray *data, quint32 len);
  qint32 readMem(quint32 addr, QByteArray *data, quint32 len);
  bool eraseFlash(quint32 len);

private slots:
  quint8 checkSum(const quint8 *data, quint8 length) const;

private:
  void prepareCmd(QByteArray *cmdBuf, quint8 cmd) const;
  QUsbDevice *mUsb;

signals:
  void connectionResult(bool result);
  void timeElapsed(int time);
};

#endif // BOOTLOADER_H
