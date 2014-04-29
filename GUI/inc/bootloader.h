#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <QObject>
#include <compat.h>
#include <QString>
#include <QtEndian>
#include <QTimer>
#include <QThread>
#include <QtUsb>

#define MAGIC (quint16)0xAFEB
#define MAGIC1 (quint8)0xAF
#define MAGIC2 (quint8)0xEB

#define MASK_CMD (quint8)0xA0
#define MASK_REPLY_OK (quint8)0x50
#define MASK_REPLY_ERR (quint8)0x70

#define CMD_ERASE (quint8)0x01
#define CMD_READ (quint8)0x02
#define CMD_WRITE (quint8)0x03
#define CMD_RESET (quint8)0x04
#define CMD_GET_FLAGS (quint8)0x05
#define CMD_WAKE (quint8)0x06
#define CMD_GET_VERSION (quint8)0x07
#define CMD_GET_SIZE (quint8)0x08

#define FLAG_OK (quint8)0x01
#define FLAG_IWDRST (quint8)0x02
#define FLAG_SFTRST (quint8)0x03

#define ERASE_OK (quint8)0x10

class Bootloader : public QObject
{
    Q_OBJECT
public:
    explicit Bootloader(QObject *parent = 0);
    ~Bootloader();
    
public slots:
    bool connect();
    bool disconnect();
    quint8 getFlags();
    quint16 getVersion();
    qint32 writeFlash(quint32 addr, const QByteArray *data, quint32 len);
    qint32 readMem(quint32 addr, QByteArray *data, quint32 len);
    bool eraseFlash(quint32 len);
    bool reset(void);
    bool isConnected(void);
    bool sendWake(void);
    void abortConnect(void);

private slots:
    quint8 checkSum(const quint8 *data, quint8 length);

private:
    QUsb *mUsb;
    bool mConnected;
    bool mAbortConnect;
    QThread mThread;

signals:
    void connectionResult(bool result);
    void timeElapsed(int time);
    
};

#endif // BOOTLOADER_H
