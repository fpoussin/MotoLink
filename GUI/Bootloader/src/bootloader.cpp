#include "bootloader.h"

Bootloader::Bootloader(QObject *parent) :
    QObject(parent)
{

    this->m_connected = false;
    this->usb = new QUsb;
}

bool Bootloader::connect()
{
    QByteArray tmp;

    if (this->usb->open() < 0) {
        return false;
    }
    /* Clean buffer */
    this->usb->read(&tmp, 256);
    this->m_connected = true;

    this->sendWake();
    this->getFlags();

    return true;
}

bool Bootloader::disconnect()
{
    this->usb->close();
    this->m_connected = false;

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

    this->usb->write(&send, send.size());
    this->usb->read(&recv, 2);

    if (recv.size() > 1)
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

    send.append(buf_addr[0]);
    send.append(buf_addr[1]);
    send.append(buf_addr[2]);
    send.append(buf_addr[3]);
    send.append(data->constData(), data->size());
    send.insert(3, send.size()+2);
    send.append(checkSum((quint8*)send.constData(), send.size()));

    qint32 wr = this->usb->write(&send, send.size());

    usleep(30000);
    this->usb->read(&recv, 1);

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
    send.append(buf_addr[0]);
    send.append(buf_addr[1]);
    send.append(buf_addr[2]);
    send.append(buf_addr[3]);
    send.append(buf_len[0]);
    send.append(buf_len[1]);
    send.append(buf_len[2]);
    send.append(buf_len[3]);
    send.insert(3, send.size()+2);
    send.append(checkSum((quint8*)send.constData(), send.size()));

    this->usb->write(&send, send.size());

    return this->usb->read(data, len);
}

bool Bootloader::eraseFlash(quint32 len)
{
    QByteArray send, recv;
    quint8 buf_len[4];

    qToLittleEndian(len, buf_len);

    send.append(MAGIC1);
    send.append(MAGIC2);
    send.append(MASK_CMD | CMD_ERASE);
    send.append(buf_len[0]);
    send.append(buf_len[1]);
    send.append(buf_len[2]);
    send.append(buf_len[3]);
    send.insert(3, send.size()+2);
    send.append(checkSum((quint8*)send.constData(), send.size()));

    this->usb->write(&send, send.size());

    usleep(1000000);

    this->usb->read(&recv, 2);

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

    this->usb->write(&send, send.size());
    this->usb->read(&recv, 1);

    return true;
}

bool Bootloader::isConnected()
{
    return this->m_connected;
}

bool Bootloader::sendWake()
{
    QByteArray send, recv;

    send.append(MAGIC1);
    send.append(MAGIC2);
    send.append(MASK_CMD | CMD_WAKE);
    send.insert(3, send.size()+2);
    send.append(checkSum((quint8*)send.constData(), send.size()));

    this->usb->write(&send, send.size());
    this->usb->read(&recv, 1);

    return true;
}

quint8 Bootloader::checkSum(const quint8 *data, quint8 length)
{
    quint8 i;
    quint8 sum = 0;

    for (i = 0; i < length; i++)
      sum += data[i];

    return sum;
}
