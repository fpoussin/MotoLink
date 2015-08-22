#include "bootloader.h"

Bootloader::Bootloader(QUsbDevice *usb, QObject *parent) :
    QObject(parent)
{
    mUsb = usb;
}

Bootloader::~Bootloader()
{

}

quint8 Bootloader::getFlags()
{
    _WAIT_USB_
    QByteArray send, recv;

    this->prepareCmd(&send, CMD_GET_FLAGS);

    mUsb->write(&send, send.size());
    mUsb->read(&recv, 2);

    if (recv.size() > 1 && recv.at(0) == (MASK_REPLY_OK | CMD_GET_FLAGS))
        return recv.at(1);

    return 0;
}

bool Bootloader::boot()
{
    _WAIT_USB_
    QByteArray send, recv;
    this->prepareCmd(&send, CMD_BOOT);

    mUsb->write(&send, send.size());
    mUsb->read(&recv, 1);

    if (recv.size() > 0 && recv.at(0) == (MASK_REPLY_OK | CMD_BOOT))
        return true;

    return false;
}

qint32 Bootloader::writeFlash(quint32 addr, const QByteArray *data, quint32 len)
{
    //WAIT_USB
    QByteArray send, recv;
    quint8 buf_len[4];
    qToLittleEndian(len, buf_len);
    quint8 buf_addr[4];
    qToLittleEndian(addr, buf_addr);

    send.append((char*)buf_addr, 4);
    send.append(data->constData(), data->size());
    this->prepareCmd(&send, CMD_WRITE);

    qint32 wr = mUsb->write(&send, send.size());

    mUsb->read(&recv, 1);

    if (recv.size() < 1)
        return -1;

    if (!(recv.at(0) == (MASK_REPLY_OK | CMD_WRITE)))
        return -1;

    return wr;
}

qint32 Bootloader::readMem(quint32 addr, QByteArray *data, quint32 len)
{
    //WAIT_USB
    QByteArray send, recv;
    quint8 buf_len[4];
    qToLittleEndian(len, buf_len);
    quint8 buf_addr[4];
    qToLittleEndian(addr, buf_addr);

    send.append((char*)buf_addr, 4);
    send.append((char*)buf_len, 4);
    this->prepareCmd(&send, CMD_READ);

    mUsb->write(&send, send.size());
    qint32 cnt = mUsb->read(&recv, len+1);

    if (!(recv.at(0) == (MASK_REPLY_OK | CMD_READ)))
        return -1;

    recv.remove(0, 1);
    *data = recv;

    return cnt-1;
}

bool Bootloader::eraseFlash(quint32 len)
{
    _WAIT_USB_
    QByteArray send, recv;
    quint8 buf_len[4];

    qToLittleEndian(len, buf_len);

    send.append((char*)buf_len, 4);
    this->prepareCmd(&send, CMD_ERASE);

    mUsb->write(&send, send.size());

     QThread::usleep(13*len);

    mUsb->read(&recv, 1);

    if (recv.size() > 0)
        return ((recv.at(0) == (MASK_REPLY_OK | CMD_ERASE)));

    return false;
}

quint8 Bootloader::checkSum(const quint8 *data, quint8 length) const
{
    quint8 i;
    quint8 sum = 0;

    for (i = 0; i < length; i++)
      sum += data[i];

    return sum;
}

void Bootloader::prepareCmd(QByteArray *cmdBuf, quint8 cmd) const
{
    cmdBuf->insert(0, MAGIC1);
    cmdBuf->insert(1, MAGIC2);
    cmdBuf->insert(2, MASK_CMD | cmd);
    cmdBuf->insert(3, cmdBuf->size()+2); // +2 = The byte we are adding now, and the checksum.
    cmdBuf->append(checkSum((quint8*)cmdBuf->constData(), cmdBuf->size()));
}
