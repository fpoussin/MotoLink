#ifndef MOTOLINK_H
#define MOTOLINK_H

#include <QObject>
#include <QUsb>
#include "bootloader.h"

class Motolink : public QObject
{
    Q_OBJECT
public:
    explicit Motolink(QUsb *usb, QObject *parent = 0);

signals:

public slots:
    void connect(void);
    void disconnect(void);


private:
    QUsb *mUsb;
    Bootloader *mBtl;
    QString mGuid;
    quint16 mPid;
    quint16 mVid;
};

#endif // MOTOLINK_H
