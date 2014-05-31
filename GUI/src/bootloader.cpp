#include "bootloader.h"

Bootloader::Bootloader(QUsb *usb, QObject *parent) :
    QObject(parent)
{
    moveToThread(&mThread);
    mThread.start();

    mUsb = usb;
}

Bootloader::~Bootloader()
{
    mThread.exit();
    if(!mThread.wait(1000))
    {
      mThread.terminate();
      mThread.wait();
    }
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

    _usleep(12*len);

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

quint8 Bootloader::checkSum(const quint8 *data, quint8 length) const
{
    quint8 i;
    quint8 sum = 0;

    for (i = 0; i < length; i++)
      sum += data[i];

    return sum;
}
