#ifndef MOTOLINK_H
#define MOTOLINK_H

#include <QObject>
#include <QUsb>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QDataStream>
#include <QList>
#include "bootloader.h"

#define _LOCK_ QMutexLocker locker(&mMutex);
#define _UNLOCK_ locker.unlock();

typedef struct {
    float vAn7; /* VBAT */
    float vAn8; /* TPS */
    float vAn9; /* AFR */
    float tps;
    float afr;
    quint16 knock_value;
    quint16 knock_freq;
    quint16 rpm;
    quint16 freq1;
    quint16 freq2;
    quint8 row;
    quint8 col;
} sensors_data_t ;

typedef struct {
    QString name;
    float cpu;
    bool active;
} mtl_task_t;

typedef struct {
  quint8 iname;
  quint8 type;
  quint16 value;
} mtl_value_t;

typedef QList<mtl_task_t> TaskList;
typedef QList<mtl_value_t> ValueList;

class Motolink : public QObject
{
    Q_OBJECT
public:
    explicit Motolink(QObject *parent = 0);
    ~Motolink();
    quint8 getBtlFlags(void) { _LOCK_ quint8 tmp = mBtl->getFlags(); _UNLOCK_ return tmp; }
    bool boot() { _LOCK_ bool tmp = mBtl->boot(); _UNLOCK_ return tmp; }
    inline bool isConnected(void) { return mConnected; }
    inline const TaskList * getMonitoring(void) { return &mMonitoring; }
    inline const QByteArray * getKnockSpectrum(void) { return &mKnockData; }
    inline const quint8 * getAFRTable(void) { return (quint8 *)&mAFRTable; }
    inline const quint8 * getKnockTable(void) { return (quint8 *)&mKnockTable; }

    // Sensors (Get)
    inline float getTPS(void) { return mSensors.tps ;}
    inline float getAFR(void) { return mSensors.afr ;}
    inline quint16 getRPM(void) { return mSensors.rpm ;}
    inline quint16 getRPMHz(void) { return mSensors.freq1 ;}
    inline quint16 getSpeedHz(void) { return mSensors.freq2 ;}
    inline quint16 getKnockPeakFreq(void) { return mSensors.knock_freq ;}
    inline quint16 getKnockValue(void) { return mSensors.knock_value ;}
    inline float getVBAT(void) { return mSensors.vAn7 ;}
    inline float getVTPS(void) { return mSensors.vAn8 ;}
    inline float getVAFR(void) { return mSensors.vAn9 ;}
    inline quint8 getColumn(void) { return mSensors.col ;}
    inline quint8 getRow(void) { return mSensors.row ;}

    // Settings
    // Get
    inline quint16 getKnockFreq(void) { return mSettings.knockFreq ;}
    inline quint16 getKnockRatio(void) { return mSettings.knockRatio ;}
    inline float getTPSMinV(void) { return mSettings.tpsMinV / 1000.0 ;}
    inline float getTPSMaxV(void) { return mSettings.tpsMaxV / 1000.0 ;}
    inline float getFuelMinTh(void) { return mSettings.fuelMinTh / 1000.0 ;}
    inline float getAFRMinVal(void) { return mSettings.AfrMinVal / 1000.0 ;}
    inline float getAFRMaxVal(void) { return mSettings.AfrMaxVal / 1000.0 ;}
    inline float getAFROffset(void) { return mSettings.AfrOffset / 1000.0 ;}
    inline bool getFunctionAFR_Disabled(void) { return mSettings.functions & FUNC_AFR_DISA ;}
    inline bool getFunctionAFR_Analog(void) { return mSettings.functions & FUNC_AFR_AN ;}
    inline bool getFunctionAFR_MTS(void) { return mSettings.functions & FUNC_AFR_MTS ;}
    inline bool getFunctionRecording(void) { return mSettings.functions & FUNC_RECORD ;}

    // Set
    inline void setTPSMinV(float v) { mSettings.tpsMinV = v * 1000.0 ;}
    inline void setTPSMaxV(float v) { mSettings.tpsMaxV = v * 1000.0 ;}
    inline void setAFRMinVal(float v) { mSettings.AfrMinVal = v * 1000.0 ;}
    inline void setAFRMaxVal(float v) { mSettings.AfrMaxVal = v * 1000.0 ;}
    inline void setAFROffset(float v) { mSettings.AfrOffset = v * 1000.0 ;}
    inline void setFunctionAFR_Disabled(void) { mSettings.functions &= ~(FUNC_AFR_AN |FUNC_AFR_MTS);
                                                mSettings.functions |= FUNC_AFR_DISA ;}
    inline void setFunctionAFR_Analog(void) {   mSettings.functions &= ~(FUNC_AFR_DISA |FUNC_AFR_MTS);
                                                mSettings.functions |= FUNC_AFR_AN ;}
    inline void setFunctionAFR_MTS(void) {      mSettings.functions &= ~(FUNC_AFR_DISA |FUNC_AFR_AN);
                                                mSettings.functions |= FUNC_AFR_MTS ;}
    inline void setFunctionRecord(bool on) {    if (on) mSettings.functions |= FUNC_RECORD;
                                                else mSettings.functions &= ~FUNC_RECORD ;}

public slots:
    bool usbConnect(void);
    bool usbProbeConnect(void);
    bool usbDisconnect(void);
    bool resetDevice(void);
    bool bootAppIfNeeded(void);

    quint8 getMode(void);
    QString getVersion(void);
    bool readSensors(void);
    bool readMonitoring(void);
    bool readKnockSpectrum(void);
    bool readTables(void);
    bool writeTablesHeaders(void);
    bool readSerialData(void);

    bool sendWake();

    void startUpdate(QByteArray *data);
    void haltTransfer(void);
    bool sendFirmware(QByteArray *data);
    bool verifyFirmware(QByteArray *data);

    bool writeSettings();
    bool readSettings();

    bool writeTablesHeaders(const quint8 *rows, const quint8 *cols);

    bool clearCell(uint tableId, int row, int col);
    bool clearTables(void);

    void clearUsb(void);

signals:
    void transferProgress(int p);
    void signalStatus(const QString &s);
    void signalLock(bool enabled);
    void updateDone(void);
    void communicationError(const QString & msg);

    void connectionProgress(int progress);
    void connectionResult(bool result);
    void timeElapsed(int time);

    void receivedSensors();
    void receivedSettings();
    void receivedMonitoring(const TaskList * monitoring);
    void receivedKockSpectrum(const QByteArray * data);
    void receivedTables(const quint8 * AFR, const quint8 * Knock);
    void receivedSerialData(QByteArray data);

private slots:
    quint8 checkSum(const quint8 *data, quint8 length) const;
    void setupConnections(void);

private:
    void prepareCmd(QByteArray* cmdBuf, quint8 cmd) const;
    bool sendSimpleCmd(quint8 cmd);
    bool sendCmd(QByteArray* send, QByteArray *recv, uint len, quint8 cmd);
    int readMore(QByteArray *recv, uint len);
    void printError(quint8 reply);
    QMutex mMutex;
    QThread *mThread;
    QUsbDevice *mUsb;
    Bootloader *mBtl;
    bool mConnected;
    bool mAbortConnect;
    QString mGuid;
    quint16 mPid;
    quint16 mVid;
    bool mStopTranfer;
    sensors_data_t mSensors;
    settings_t mSettings;
    version_t mVersion;
    TaskList mMonitoring;
    QStringList mNames;
    QByteArray mKnockData;
    quint8 mAFRTable[11][16];
    quint8 mKnockTable[11][16];
    quint8 mTablesRows[11];
    quint8 mTablesColumns[16];
};

#endif // MOTOLINK_H
