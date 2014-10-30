#include "motolink.h"

Motolink::Motolink(QObject *parent) :
    QObject(parent)
{
    mUsb = new QUsb;
    mThread = new QThread;

    mGuid = "656d69a0-4f42-45c4-b92c-bffa3b9e6bd1";
    mVid = 0x0483;
    mPid = 0xABCD;

    mUsb->setGuid(mGuid);
    mUsb->setDeviceIds(mPid, mVid);
    mUsb->setEndPoints(0x83, 0x03);
    mUsb->setTimeout(300);

    mBtl = new Bootloader(mUsb);

    mConnected = false;
    mAbortConnect = false;

    //mUsb->setDebug(true);

    this->moveToThread(mThread);
    mThread->start();
}

Motolink::~Motolink()
{
    mThread->exit();
    if (!mThread->wait(1000)) {
        mThread->terminate();
        mThread->wait();
    }
    delete mThread;
    delete mBtl;
    this->usbDisconnect();
    delete mUsb;
}

bool Motolink::usbConnect()
{
    if (mConnected)
        return true;
    _LOCK_
    mConnected = (mUsb->open() == 0);
    _UNLOCK_

    if (mConnected)
        this->sendWake();

    return mConnected;
}

bool Motolink::usbProbeConnect()
{
    _LOCK_
    QByteArray tmp;
    QElapsedTimer timer;
    qint32 ret = -1;
    mAbortConnect = false;
    emit connectionProgress(0);

    timer.start();
    while (timer.elapsed() < 10000)
    {
        qDebug("Probing...");

        emit timeElapsed(timer.elapsed());
        ret = mUsb->open();
        if (ret >= 0 || mAbortConnect) {
            break;
        }
        _usleep(100000);
        emit connectionProgress(timer.elapsed()/100);
    }

    if (ret < 0) {
        qDebug("Probing Failed");
        emit connectionResult(false);
        mConnected = false;
        emit connectionProgress(0);
        return false;
    }

    /* Clean buffer */
    mUsb->read(&tmp, 256);
    mConnected = true;

    qDebug("Probing success");

    //_usleep(200000);
    //this->sendWake();

    emit connectionProgress(100);
    emit connectionResult(true);

    return true;
}

bool Motolink::usbDisconnect(void)
{
    if (!mConnected) {
        return true;
    }

    _LOCK_
    mUsb->close();
    mConnected = false;

    return true;
}

quint8 Motolink::getMode(void)
{
    _WAIT_USB_
    _LOCK_
    QByteArray send, recv;
    prepareSimpleCmd(&send, CMD_GET_MODE);

    if (mUsb->write(&send, send.size()) < send.size())
    {
        return 0;
    }
    mUsb->read(&recv, 2);

    if (recv.size() > 1 && recv.at(0) == (MASK_REPLY_OK | CMD_GET_MODE))
        return recv.at(1) & ~MASK_REPLY_OK;

    return 0;
}


quint16 Motolink::getVersion()
{
    _WAIT_USB_
    _LOCK_
    QByteArray send, recv;
    prepareSimpleCmd(&send, CMD_GET_VERSION);

    if (mUsb->write(&send, send.size()) < send.size())
    {
        return 0;
    }
    mUsb->read(&recv, 3);

    if (recv.size() > 2 && recv.at(0) == (MASK_REPLY_OK | CMD_GET_VERSION))
        return recv.at(1) + (recv.at(2)*256);

    return 0;
}

bool Motolink::getSensors(QByteArray* data)
{
    _WAIT_USB_
    _LOCK_
    QByteArray send, recv;
    prepareSimpleCmd(&send, CMD_GET_SENSORS);

    if (mUsb->write(&send, send.size()) < send.size())
    {
        return 0;
    }
    mUsb->read(&recv, sizeof(sensors_t)+1);

    if ((size_t)recv.size() > sizeof(sensors_t) && recv.at(0) == (MASK_REPLY_OK | CMD_GET_SENSORS))
    {
        *data = recv.remove(0, 1);
        memcpy((void*)&mSensors, (void*)data->constData(), sizeof(sensors_t));
        emit sendSensors(data);
        return true;
    }

    return false;
}

bool Motolink::getMonitoring(QByteArray *data)
{
    _WAIT_USB_
    _LOCK_
    QByteArray send, recv;
    prepareSimpleCmd(&send, CMD_GET_MONITOR);

    if (mUsb->write(&send, send.size()) < send.size())
    {
        return 0;
    }
    mUsb->read(&recv, sizeof(monitor_t)+1);

    if ((size_t)recv.size() > sizeof(monitor_t) && recv.at(0) == (MASK_REPLY_OK | CMD_GET_MONITOR))
    {
        *data = recv.remove(0, 1);
        memcpy((void*)&mMonitoring, (void*)data->constData(), sizeof(monitor_t));
        emit sendMonitoring(data);
        return true;
    }

    return false;
}

bool Motolink::getKnockSpectrum(QByteArray *data)
{
    _WAIT_USB_
    _LOCK_
    QByteArray send, recv;
    prepareSimpleCmd(&send, CMD_GET_FFT);

    if (mUsb->write(&send, send.size()) < send.size())
    {
        return 0;
    }
    mUsb->read(&recv, SPECTRUM_SIZE+1);

    if ((size_t)recv.size() > SPECTRUM_SIZE && recv.at(0) == (MASK_REPLY_OK | CMD_GET_FFT))
    {
        *data = recv.remove(0, 1);
        emit sendKockSpectrum(data);
        return true;
    }

    return false;
}

bool Motolink::sendWake()
{
    _WAIT_USB_
    _LOCK_
    QByteArray send, recv;
    prepareSimpleCmd(&send, CMD_WAKE);

    if (mUsb->write(&send, send.size()) < send.size())
    {
        return 0;
    }
    mUsb->read(&recv, 1);

    return recv.at(0) == (MASK_REPLY_OK | CMD_WAKE);
}

void Motolink::startUpdate(QByteArray *data)
{
    this->sendFirmware(data);
    this->verifyFirmware(data);

    emit updateDone();
}

bool Motolink::resetDevice()
{
    _WAIT_USB_
    _LOCK_
    QByteArray send, recv;
    prepareSimpleCmd(&send, CMD_RESET);

    if (mUsb->write(&send, send.size()) < send.size())
    {
        return 0;
    }
    mUsb->read(&recv, 1);

    return recv.at(0) == (MASK_REPLY_OK | CMD_RESET);
}

bool Motolink::bootAppIfNeeded()
{
    if (!mConnected)
        this->usbConnect();

    if (this->getMode() == MODE_BL)
    {
        mBtl->boot();
        _usleep(100000);
        this->usbDisconnect();
        _usleep(2000000);
        this->usbConnect();
        return true;
    }
    return false;
}

quint8 Motolink::checkSum(const quint8 *data, quint8 length) const
{
    quint8 i;
    quint8 sum = 0;

    for (i = 0; i < length; i++)
      sum += data[i];

    return sum;
}

void Motolink::setupConnections()
{
    //QObject::connect(mTft, SIGNAL(updateDone()), mBtl, SLOT(disconnect()));
}

void Motolink::prepareSimpleCmd(QByteArray *cmdBuf, quint8 cmd) const
{
    cmdBuf->clear();
    cmdBuf->append(MAGIC1);
    cmdBuf->append(MAGIC2);
    cmdBuf->append(MASK_CMD | cmd);
    cmdBuf->insert(3, cmdBuf->size()+2);
    cmdBuf->append(checkSum((quint8*)cmdBuf->constData(), cmdBuf->size()));
}

void Motolink::haltTransfer(void)
{
    mStopTranfer = true;
    emit sendStatus(tr("Aborted"));
}

void Motolink::sendFirmware(QByteArray *data)
{
    QDataStream file(data, QIODevice::ReadOnly);
    emit sendLock(true);
    mStopTranfer = false;
    quint32 step_size = 240;
    const quint32 from = 0;
    const quint32 to = data->size();

    quint32 progress, oldprogress;
    qint32 read;
    char *buf2 = new char[step_size];

    qDebug() << tr("File size") << data->size();

    emit sendStatus(tr("Erasing..."));
    _LOCK_
    if (!mBtl->eraseFlash(data->size())) {

        emit sendStatus(tr("Erase failed"));
        qDebug() << tr("Erase failed");
        emit sendLock(false);
        return;
    }
    else {
        emit sendStatus(tr("Erase OK"));
        qDebug() <<tr( "Erase OK");
        _UNLOCK_
    }

    qDebug() << tr("Writing from") << "0x"+QString::number(from, 16) << "to" << "0x"+QString::number(to, 16);

    emit sendStatus(tr("Writing Flash"));

    progress = 0;
    for (int i=0; i<=data->size(); i+=step_size) {

        if (mStopTranfer)
            break;

        if (file.atEnd()) {
            qDebug() << tr("End Of File");
            break;
        }

        memset(buf2, 0, step_size);
        if ((read = file.readRawData(buf2, step_size)) <= 0)
            break;
        qDebug() << tr("Read") << read << tr("Bytes from disk");
        QByteArray buf(buf2, read);

        _LOCK_
        int wrote = mBtl->writeFlash(i, &buf, read);
        _UNLOCK_

        if (wrote < read){
            emit sendStatus(tr("Transfer failed"));
            qDebug() << tr("Transfer failed") << wrote << read;
            break;
        }

        oldprogress = progress;
        progress = (i*100)/data->size();
        if (progress > oldprogress) { // Push only if number has increased
            emit sendProgress(progress);
            qDebug() << tr("Progress:") << QString::number(progress)+"%";
        }

    }
    delete buf2;

    emit sendProgress(100);
    emit sendStatus(tr("Transfer done"));
    qDebug() << tr("Transfer done");

    emit sendLock(false);
}

void Motolink::verifyFirmware(QByteArray *data)
{
    QDataStream file(data, QIODevice::ReadOnly);
    emit sendLock(true);
    mStopTranfer = false;
    const quint32 buf_size = 508;
    const quint32 from = 0;
    const quint32 to = data->size();
    char buf[buf_size];
    qDebug() << tr("Reading from") << QString::number(from, 16) << tr("to") << QString::number(to, 16);
    quint32 addr, progress, oldprogress;
    QByteArray data_local, data_remote;

    emit sendStatus(tr("Verifying flash"));

    progress = 0;
    for (int i=0; i<data->size(); i+=buf_size)
    {
        if (mStopTranfer)
            break;

        int read = file.readRawData(buf, buf_size);
        data_local.setRawData(buf, read);
        qDebug() << tr("Read") << data_local.size() << tr("Bytes from disk");
        addr = i;

        if (!data_local.size()) break;
        _LOCK_
        int read_size = mBtl->readMem(addr, &data_remote, data_local.size());
        _UNLOCK_
        if (read_size < data_local.size()) { // Read same amount of data as from file.

            emit sendProgress(0);
            emit sendStatus(tr("Verification Failed"));
            qWarning() << tr("Verification Failed");
            qDebug() << tr("read") << read_size << tr("valid") << data_local.size();
            emit sendLock(false);
            return;
        }

        if (data_remote != data_local) {

            emit sendProgress(100);
            emit sendStatus(tr("Verification failed at 0x")+QString::number(addr, 16));

            QString stmp, sbuf;
            for (int b=0;b<data_local.size();b++) {
                stmp.append(QString().sprintf("%02X ", (uchar)data_local.at(b)));
                sbuf.append(QString().sprintf("%02X ", (uchar)data_remote.at(b)));
            }
            qCritical() << tr("Verification failed at 0x")+QString::number(addr, 16) <<
                           "\r\n" << tr("Expecting:") << stmp << "\r\n       " << tr("Got:") << sbuf;
            emit sendLock(false);
            return;
        }
        oldprogress = progress;
        progress = (i*100)/data->size();
        if (progress > oldprogress) { // Push only if number has increased
            emit sendProgress(progress);
            qDebug() << tr("Progress:") << QString::number(progress)+"%";
        }
        //emit sendStatus(tr("Verified ")+QString::number(i/1024)+tr(" kilobytes out of ")+QString::number(data->size()/1024));
    }

    emit sendProgress(100);
    emit sendStatus(tr("Verification OK"));
    qDebug() << tr("Verification OK");
    emit sendLock(false);
}
