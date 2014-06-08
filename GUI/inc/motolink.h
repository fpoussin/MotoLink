#ifndef MOTOLINK_H
#define MOTOLINK_H

#include <QObject>
#include <QUsb>
#include "bootloader.h"
#include "transferthread.h"

class Motolink : public QObject
{
    Q_OBJECT
public:
    explicit Motolink(QObject *parent = 0);
    ~Motolink();
    Bootloader * getBtl(void) { return mBtl; }
    TransferThread * getTft(void) { return mTft; }

public slots:
    bool usbConnect(void);
    bool probeConnect(void);
    bool usbDisconnect(void);
    bool resetDevice(void);
    bool bootAppIfNeeded(void);

    quint8 getMode(void);
    quint16 getVersion(void);
    bool sendWake();

signals:
    void connectionProgress(int progress);
    void connectionResult(bool result);
    void timeElapsed(int time);

private slots:
    quint8 checkSum(const quint8 *data, quint8 length) const;
    void setupConnection(void);

private:
    QUsb *mUsb;
    Bootloader *mBtl;
    TransferThread *mTft;
    bool mConnected;
    bool mAbortConnect;
    QString mGuid;
    quint16 mPid;
    quint16 mVid;
};

#endif // MOTOLINK_H
