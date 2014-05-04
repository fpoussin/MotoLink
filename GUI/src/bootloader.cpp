#include "bootloader.h"

Bootloader::Bootloader(QUsb *usb, QObject *parent) :
    QObject(parent)
{
    moveToThread(&mThread);
    mThread.start();

    mConnected = false;
    mAbortConnect = false;
    mUsb = usb;

    mGuid = "f0207e85-88d7-4839-bba8-bf87a5092f03";
    mPid = 0x0483;
    mVid = 0xFEDC;

    mUsb->setGuid(mGuid);
    mUsb->setDeviceIds(mPid, mVid);
}

Bootloader::~Bootloader()
{
    this->abortConnect();
    mThread.exit();
    if(!mThread.wait(1000))
    {
      mThread.terminate();
      mThread.wait();
    }
}

bool Bootloader::connect()
{
    QByteArray tmp;
    QElapsedTimer timer;
    qint32 ret = -1;
    mAbortConnect = false;

    timer.start();
    while (timer.elapsed() < 10000) {

        emit timeElapsed(timer.elapsed());
        ret = mUsb->open();
        if (ret >= 0 || mAbortConnect) {
            break;
        }
        _usleep(50000);
    }

    if (ret < 0) {
        emit connectionResult(false);
        mConnected = false;
        return false;
    }

    /* Clean buffer */
    mUsb->read(&tmp, 256);
    mConnected = true;

    this->sendWake();
    this->getFlags();

    emit connectionResult(true);

    return true;
}

bool Bootloader::disconnect()
{
    mUsb->close();
    mConnected = false;

    return true;
}

quint8 Bootloader::getFlags()
{
    QByteArray send, recv;

    send.append(MAGIC1);
    send.append(MAGIC2);
    send.append(MASK_CMD | CMD_GET_FLAGS);
    send.insert(3, send.size()+2);
    send.append(checkSum((quint8*)send.constData(), send.size()));

    mUsb->write(&send, send.size());
    mUsb->read(&recv, 2);

    if (recv.size() > 1 && recv.at(0) == MASK_REPLY_OK)
        return recv.at(1);

    return 0;
}

quint16 Bootloader::getVersion()
{
    QByteArray send, recv;

    send.append(MAGIC1);
    send.append(MAGIC2);
    send.append(MASK_CMD | CMD_GET_VERSION);
    send.insert(3, send.size()+2);
    send.append(checkSum((quint8*)send.constData(), send.size()));

    mUsb->write(&send, send.size());
    mUsb->read(&recv, 3);

    if (recv.size() > 2 && recv.at(0) == MASK_REPLY_OK)
        return recv.at(1) + (recv.at(2)*256);

    return 0;
}

qint32 Bootloader::writeFlash(quint32 addr, const QByteArray *data, quint32 len)
{
    QByteArray send, recv;
    quint8 buf_len[4];
    qToLittleEndian(len, buf_len);
    quint8 buf_addr[4];
    qToLittleEndian(addr, buf_addr);

    send.append(MAGIC1);
    send.append(MAGIC2);
    send.append(MASK_CMD | CMD_WRITE);

    send.append((char*)buf_addr, 4);
    send.append(data->constData(), data->size());
    send.insert(3, send.size()+2);
    send.append(checkSum((quint8*)send.constData(), send.size()));

    qint32 wr = mUsb->write(&send, send.size());

    _usleep(30000);
    mUsb->read(&recv, 1);

    if (recv.size() < 1)
        return -1;

    if (recv.at(0) != MASK_REPLY_OK)
        return -1;

    return wr;
}

qint32 Bootloader::readMem(quint32 addr, QByteArray *data, quint32 len)
{
    QByteArray send;
    quint8 buf_len[4];
    qToLittleEndian(len, buf_len);
    quint8 buf_addr[4];
    qToLittleEndian(addr, buf_addr);

    send.append(MAGIC1);
    send.append(MAGIC2);
    send.append(MASK_CMD | CMD_READ);
    send.append((char*)buf_addr, 4);
    send.append((char*)buf_len, 4);
    send.insert(3, send.size()+2);
    send.append(checkSum((quint8*)send.constData(), send.size()));

    mUsb->write(&send, send.size());

    return mUsb->read(data, len);
}

bool Bootloader::eraseFlash(quint32 len)
{
    QByteArray send, recv;
    quint8 buf_len[4];

    qToLittleEndian(len, buf_len);

    send.append(MAGIC1);
    send.append(MAGIC2);
    send.append(MASK_CMD | CMD_ERASE);
    send.append((char*)buf_len, 4);
    send.insert(3, send.size()+2);
    send.append(checkSum((quint8*)send.constData(), send.size()));

    mUsb->write(&send, send.size());

    _usleep(1000000);

    mUsb->read(&recv, 2);

    if (recv.size() >= 2)
        return ((recv.at(0) == MASK_REPLY_OK) && (recv.at(1) == ERASE_OK));

    return false;
}

bool Bootloader::reset()
{
    QByteArray send, recv;

    send.append(MAGIC1);
    send.append(MAGIC2);
    send.append(MASK_CMD | CMD_RESET);
    send.insert(3, send.size()+2);
    send.append(checkSum((quint8*)send.constData(), send.size()));

    mUsb->write(&send, send.size());
    mUsb->read(&recv, 1);

    return true;
}

bool Bootloader::isConnected() const
{
    return mConnected;
}

bool Bootloader::sendWake()
{
    QByteArray send, recv;

    send.append(MAGIC1);
    send.append(MAGIC2);
    send.append(MASK_CMD | CMD_WAKE);
    send.insert(3, send.size()+2);
    send.append(checkSum((quint8*)send.constData(), send.size()));

    mUsb->write(&send, send.size());
    mUsb->read(&recv, 1);

    return recv.at(0) == MASK_REPLY_OK;
}

void Bootloader::abortConnect()
{
    mAbortConnect = true;
}

quint8 Bootloader::checkSum(const quint8 *data, quint8 length) const
{
    quint8 i;
    quint8 sum = 0;

    for (i = 0; i < length; i++)
      sum += data[i];

    return sum;
}
