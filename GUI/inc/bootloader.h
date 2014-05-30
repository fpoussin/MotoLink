#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <QObject>
#include <compat.h>
#include <QString>
#include <QtEndian>
#include <QTimer>
#include <QThread>
#include <QUsb>

typedef quint8 uint8_t;
#include "protocol.h"

class Bootloader : public QObject
{
    Q_OBJECT
public:
    explicit Bootloader(QUsb *usb, QObject *parent = 0);
    ~Bootloader();
    
public slots:
    bool connect();
    bool disconnect();
    quint8 getFlags();
    quint8 getMode();
    quint16 getVersion();
    qint32 writeFlash(quint32 addr, const QByteArray *data, quint32 len);
    qint32 readMem(quint32 addr, QByteArray *data, quint32 len);
    bool eraseFlash(quint32 len);
    bool reset(void);
    bool isConnected(void) const;
    bool sendWake(void);
    void abortConnect(void);

private slots:
    quint8 checkSum(const quint8 *data, quint8 length) const;

private:
    QUsb *mUsb;
    bool mConnected;
    bool mAbortConnect;
    QThread mThread;
    QString mGuid;
    quint16 mPid;
    quint16 mVid;

signals:
    void connectionResult(bool result);
    void timeElapsed(int time);
    
};

#endif // BOOTLOADER_H
