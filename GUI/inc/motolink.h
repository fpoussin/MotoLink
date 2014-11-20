#ifndef MOTOLINK_H
#define MOTOLINK_H

#include <QObject>
#include <QUsb>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include "bootloader.h"

#define _LOCK_ QMutexLocker locker(&mMutex);
#define _UNLOCK_ locker.unlock();

class Motolink : public QObject
{
    Q_OBJECT
public:
    explicit Motolink(QObject *parent = 0);
    ~Motolink();
    quint8 getBtlFlags(void) { _LOCK_ quint8 tmp = mBtl->getFlags(); _UNLOCK_ return tmp; }
    bool boot() { _LOCK_ bool tmp = mBtl->boot(); _UNLOCK_ return tmp; }
    bool isConnected(void) { return mConnected; }
    const sensors_t * readSensors(void) { return &mSensors; }

public slots:
    bool usbConnect(void);
    bool usbProbeConnect(void);
    bool usbDisconnect(void);
    bool resetDevice(void);
    bool bootAppIfNeeded(void);

    quint8 getMode(void);
    quint16 getVersion(void);
    bool readSensors(QByteArray *data);
    bool readMonitoring(QByteArray *data);
    bool readKnockSpectrum(QByteArray *data);
    bool sendWake();

    void startUpdate(QByteArray *data);
    void haltTransfer(void);
    void sendFirmware(QByteArray *data);
    void verifyFirmware(QByteArray *data);

    bool writeSettings(settings_t *settings);
    bool readSettings(settings_t *settings);

signals:
    void sendProgress(int p);
    void sendStatus(const QString &s);
    void sendLock(bool enabled);
    void updateDone(void);

    void connectionProgress(int progress);
    void connectionResult(bool result);
    void timeElapsed(int time);

    void sendSensors(QByteArray *data);
    void sendMonitoring(QByteArray *data);
    void sendKockSpectrum(QByteArray *data);
    void sendSettings(QByteArray *data);

private slots:
    quint8 checkSum(const quint8 *data, quint8 length) const;
    void setupConnections(void);

private:
    void prepareSimpleCmd(QByteArray* cmdBuf, quint8 cmd) const;
    QMutex mMutex;
    QThread *mThread;
    QUsb *mUsb;
    Bootloader *mBtl;
    bool mConnected;
    bool mAbortConnect;
    QString mGuid;
    quint16 mPid;
    quint16 mVid;
    bool mStopTranfer;
    sensors_t mSensors;
    monitor_t mMonitoring;
};

#endif // MOTOLINK_H
