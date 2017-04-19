#include "motolink.h"

Motolink::Motolink(QObject *parent) :
    QObject(parent)
{
    mUsb = new QUsbDevice;
    mThread = new QThread;

    mGuid = "1EE57D96-67C5-4E84-9CB7-DEEC7929B8A3";
    mVid = 0x0483;
    mPid = 0xABCD;

    QtUsb::DeviceFilter filter;
    QtUsb::DeviceConfig config;

    filter.pid = mPid;
    filter.vid = mVid;
    filter.guid = mGuid;

    config.readEp = 0x84;
    config.writeEp = 0x04;
    config.alternate = 0;
    config.config = 1;
    config.interface = 1;

    mUsb->setFilter(filter);
    mUsb->setConfig(config);
    mUsb->setTimeout(300);

    mBtl = new Bootloader(mUsb);

    mConnected = false;
    mAbortConnect = false;

    mUsb->setDebug(false);

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

    if (!mConnected || !this->sendWake())
    {
        mConnected = false;
        mUsb->close();
        return false;
    }

    if (mUsb->getSpeed() != QtUsb::fullSpeed)
    {
        qWarning("Incorrect USB speed: %s",
                 mUsb->getSpeedString().toStdString().data());
        mConnected = false;
        return false;
    }

    mUsb->flush();
    return true;
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
        emit timeElapsed(timer.elapsed());
        ret = mUsb->open();
        if (ret >= 0 || mAbortConnect) {
            break;
        }
        QThread::msleep(100);
        emit connectionProgress(timer.elapsed()/100);
    }

    if (ret < 0) {
        qWarning("Probing Failed");
        emit connectionResult(false);
        mConnected = false;
        emit connectionProgress(0);
        return false;
    }

    if (mUsb->getSpeed() != QtUsb::fullSpeed)
    {
        qWarning("Incorrect USB speed: %s",
                 mUsb->getSpeedString().toStdString().data());
        emit connectionResult(false);
        mConnected = false;
        emit connectionProgress(0);
        return false;
    }

    /* Clean buffer */
    //mUsb->read(&tmp, 256);
    mUsb->flush();

    _UNLOCK_
    if (!this->sendWake())
    {
        qWarning("Connection Failed");
        return false;
    }

    mConnected = true;

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
    QByteArray send, recv;
    this->prepareCmd(&send, CMD_GET_MODE);

    if (this->sendCmd(&send, &recv, 1, CMD_GET_MODE))
        return recv.at(0);

    return 0;
}

QString Motolink::getVersion(quint8 idx)
{
    QByteArray send, recv;
    this->prepareCmd(&send, CMD_GET_VERSION);

    if (this->sendCmd(&send, &recv, sizeof(version_t) * 2, CMD_GET_VERSION))
    {
        memcpy(mVersion, recv.constData(), sizeof(version_t) * 2);
        return QString("%1.%2.%3").arg(mVersion[idx].major).arg(mVersion[idx].minor).arg(mVersion[idx].patch);
    }
    return tr("Error");
}

bool Motolink::readSensors(void)
{
    QByteArray send, recv;
    this->prepareCmd(&send, CMD_GET_SENSORS);


    if (this->sendCmd(&send, &recv, sizeof(sensors_t), CMD_GET_SENSORS))
    {
        const sensors_t * sensors =  (sensors_t *)recv.constData();

        mSensors.vAn1 = sensors->an1/1000.0; /* VBAT */
        mSensors.vAn2 = sensors->an2/1000.0; /* TPS */
        mSensors.vAn3 = sensors->an3/1000.0; /* AFR */
        mSensors.tps = sensors->tps/2.0;
        mSensors.rpm = sensors->rpm;
        mSensors.freq1  = sensors->freq1;
        mSensors.freq2  = sensors->freq2;
        mSensors.knock_value = sensors->knock_value;
        mSensors.knock_freq = sensors->knock_freq*100;
        mSensors.afr = sensors->afr/10.0;
        mSensors.row = sensors->cell.row;
        mSensors.col = sensors->cell.col;
        emit receivedSensors();
        return true;
    }

    return false;
}

bool Motolink::readMonitoring(void)
{
    QByteArray send, recv;
    this->prepareCmd(&send, CMD_GET_MONITOR);
    int toRead = 0;
    mtl_task_t task;
    QString taskName;
    quint16 taskCpu;

    const quint16 maskUsage = 0x3FFF; /* Remove thread state in last 2 bits */
    const quint16 maskState = 0x8000; /* Thread state */

    if (this->sendCmd(&send, &recv, 0, CMD_GET_MONITOR))
    {
        recv.clear();
        mMonitoring.clear();
        _LOCK_
        while (this->readMore(&recv, 1) > 0)
        {
            toRead = (uchar)recv.at(0);
            recv.clear();

            if (toRead == 0)
            {
                break;
            }

            // Percentage
            int pct_size = this->readMore(&recv, 2);
            if (pct_size != 2)
            {
                return false;
            }
            taskCpu = *((quint16*)recv.constData());
            recv.clear();

            // Name
            int name_size = this->readMore(&recv, toRead);
            if (name_size != toRead)
            {
                return false;
            }
            taskName = recv.constData();
            recv.clear();

            task.cpu = (taskCpu & maskUsage) / 100.0;
            task.name = taskName;
            task.active = taskCpu & maskState;

            //qDebug() << task.name << task.cpu;

            mMonitoring.append(task);
        }
        emit receivedMonitoring(&mMonitoring);
        return true;
    }

    return false;
}

bool Motolink::readKnockSpectrum(void)
{
    QByteArray send, recv;
    this->prepareCmd(&send, CMD_GET_FFT);

    if (this->sendCmd(&send, &recv, SPECTRUM_SIZE, CMD_GET_FFT))
    {
        mKnockData = recv;
        emit receivedKockSpectrum(&mKnockData);
        return true;
    }

    qWarning("getKnockSpectrum Error!");

    return false;
}

bool Motolink::readTables(void)
{
    if (!mConnected)
        return false;

    QByteArray send, recv;
    int size = sizeof(mAFRTable)+sizeof(mKnockTable);
    this->prepareCmd(&send, CMD_GET_TABLES);

    if (this->sendCmd(&send, &recv, size, CMD_GET_TABLES))
    {
        memcpy((void*)mAFRTable, (void*)recv.constData(), sizeof(mAFRTable));
        memcpy((void*)mKnockTable, (void*)(recv.constData()+sizeof(mAFRTable)), sizeof(mKnockTable));
        emit receivedTables((quint8*)mAFRTable, (quint8*)mKnockTable);
        return true;
    }

    return false;
}

bool Motolink::writeTablesHeaders()
{
    if (!mConnected)
        return false;

    QByteArray send, recv;
    send.append((char*)mTablesRows, sizeof(mTablesRows));
    send.append((char*)mTablesColumns, sizeof(mTablesColumns));
    this->prepareCmd(&send, CMD_SET_TABLES_HEADERS);

    return this->sendCmd(&send, &recv, 0, CMD_SET_TABLES_HEADERS);
}

bool Motolink::readSerialData()
{
    if (!mConnected)
        return false;

    QByteArray send, recv;
    int size = 256;
    this->prepareCmd(&send, CMD_GET_SERIAL_DATA);

    if (this->sendCmd(&send, &recv, size, CMD_GET_SERIAL_DATA))
    {
        emit receivedSerialData(recv);
        return true;
    }

    return false;
}

bool Motolink::readSettings()
{
    if (!mConnected)
        return false;

    QByteArray send, recv;
    this->prepareCmd(&send, CMD_GET_SETTINGS);

    if (this->sendCmd(&send, &recv, sizeof(settings_t), CMD_GET_SETTINGS))
    {
        memcpy(&mSettings, recv.constData(), recv.size());
        emit receivedSettings();
        return true;
    }

    return false;
}

bool Motolink::writeSettings()
{
    if (!mConnected)
        return false;

    QByteArray send, recv;
    send.append((char*)&mSettings, sizeof(settings_t));
    this->prepareCmd(&send, CMD_SET_SETTINGS);

    if (this->sendCmd(&send, &recv, 0, CMD_SET_SETTINGS))
    {
        return true;
    }

    return false;
}

bool Motolink::writeTablesHeaders(const quint8 *rows, const quint8 *cols)
{
    memcpy(mTablesRows, rows, sizeof(mTablesRows));
    memcpy(mTablesColumns, cols, sizeof(mTablesColumns));

    return this->writeTablesHeaders();
}

bool Motolink::clearCell(uint tableId, int row, int col)
{
    if (!mConnected)
        return false;

    QByteArray send, recv;
    cell_t cell = {(uint8_t)row, (uint8_t)col};
    send.append(tableId&0xFF);
    send.append((char*)&cell, sizeof(cell));
    this->prepareCmd(&send, CMD_CLEAR_CELL);

    return this->sendCmd(&send, &recv, 0, CMD_CLEAR_CELL);
}

bool Motolink::clearTables()
{
    return this->sendSimpleCmd(CMD_CLEAR_TABLES);
}

void Motolink::clearUsb()
{
    if (this->mConnected)
        this->mUsb->flush();
}

bool Motolink::sendWake()
{
    return this->sendSimpleCmd(CMD_WAKE);
}

void Motolink::startUpdate(QByteArray *data)
{
    if (this->sendFirmware(data))
        this->verifyFirmware(data);

    emit updateDone();
}

bool Motolink::resetDevice()
{
    return this->sendSimpleCmd(CMD_RESET);
}

bool Motolink::bootAppIfNeeded()
{
    if (!mConnected)
        this->usbConnect();

    if (this->getMode() == MODE_BL)
    {
        mBtl->boot();
        QThread::msleep(100);
        this->usbDisconnect();
        QThread::msleep(2000);
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

void Motolink::prepareCmd(QByteArray *cmdBuf, quint8 cmd) const
{
    cmdBuf->insert(0, MAGIC1);
    cmdBuf->insert(1, MAGIC2);
    cmdBuf->insert(2, MASK_CMD | cmd);
    cmdBuf->insert(3, cmdBuf->size()+2); // +2 = The byte we are adding now, and the checksum.
    cmdBuf->append(checkSum((quint8*)cmdBuf->constData(), cmdBuf->size()));
}

bool Motolink::sendSimpleCmd(quint8 cmd)
{
    _WAIT_USB_
    _LOCK_
    bool result = false;
    QByteArray send, recv;
    this->prepareCmd(&send, cmd);

    if (mUsb->write(&send, send.size()) != send.size())
    {
        return 0;
    }
    if (mUsb->read(&recv, 1) > 0)
    {
        result = recv.at(0) == (MASK_REPLY_OK | cmd);
        if (!result)
            this->printError(recv.at(0));
    }
    return result;
}

bool Motolink::sendCmd(QByteArray *send, QByteArray *recv, uint len, quint8 cmd)
{
    _WAIT_USB_
    _LOCK_
    bool result;
    recv->clear();

    if (mUsb->write(send, send->size()) < send->size())
    {
        return 0;
    }

    mUsb->read(recv, 1); // Read command result first
    result = recv->at(0) == (MASK_REPLY_OK | cmd);
    if (!result)
        this->printError(recv->at(0));

    recv->clear();
    if (len)
        mUsb->read(recv, len); // Read actual command data

    return result && (uint)recv->size() == len;
}

int Motolink::readMore(QByteArray *recv, uint len)
{
   return mUsb->read(recv, len);
}

void Motolink::printError(quint8 reply)
{
    QString error;
    if (reply & MASK_DECODE_ERR)
    {
        if (reply & 1)
            error.append("Header read error");
        else if (reply & 2)
            error.append("Header decode error");
        else if (reply & 3)
            error.append("Data read error");
        else if (reply & 4)
            error.append("Checksum error");
        else if (reply & 5)
            error.append("Unknown command");

        emit communicationError(error);
    }
    else if (reply & MASK_CMD_ERR)
    {
        error.append("Command error: ");

        if ((reply & MASK_CMD_PART) == CMD_GET_MONITOR)
            error.append("Read monitor");
        else if ((reply & MASK_CMD_PART) == CMD_GET_SENSORS)
            error.append("Read sensors");
        else if ((reply & MASK_CMD_PART) == CMD_GET_SETTINGS)
            error.append("Read settings");
        else if ((reply & MASK_CMD_PART) == CMD_GET_TABLES)
            error.append("Read tables");
        else if ((reply & MASK_CMD_PART) == CMD_GET_TABLES_HEADERS)
            error.append("Read tables headers");
        else if ((reply & MASK_CMD_PART) == CMD_SET_TABLES_HEADERS)
            error.append("Write tables headers");
        else if ((reply & MASK_CMD_PART) == CMD_GET_FFT)
            error.append("Read fft");
        else if ((reply & MASK_CMD_PART) == CMD_GET_VERSION)
            error.append("Read version");
        else if ((reply & MASK_CMD_PART) == CMD_CLEAR_CELL)
            error.append("Clear cell");
        else if ((reply & MASK_CMD_PART) == CMD_CLEAR_TABLES)
            error.append("Clear tables");
        else
            error.append(QString::number(reply));

        emit communicationError(error);
    }
}

void Motolink::haltTransfer(void)
{
    mStopTranfer = true;
    emit signalStatus(tr("Aborted"));
}

bool Motolink::sendFirmware(QByteArray *data)
{
    QDataStream file(data, QIODevice::ReadOnly);
    emit signalLock(true);
    mStopTranfer = false;
    quint32 step_size = 240;

    quint32 progress, oldprogress;
    qint32 read;
    char *buf2 = new char[step_size];

    emit signalStatus(tr("Erasing..."));
    _LOCK_
    if (!mBtl->eraseFlash(data->size())) {

        emit signalStatus(tr("Erase failed"));
        emit signalLock(false);
        return false;
    }
    else {
        emit signalStatus(tr("Erase OK"));
        _UNLOCK_
    }

    emit signalStatus(tr("Writing Flash"));

    progress = 0;
    for (int i=0; i<=data->size(); i+=step_size) {

        if (mStopTranfer)
            break;

        if (file.atEnd()) {
            break;
        }

        memset(buf2, 0, step_size);
        if ((read = file.readRawData(buf2, step_size)) <= 0)
            break;
        QByteArray buf(buf2, read);

        _LOCK_
        int wrote = mBtl->writeFlash(i, &buf, read);
        _UNLOCK_

        if (wrote < read){
            emit signalStatus(tr("Transfer failed"));
            qWarning() << tr("Transfer failed") << wrote << read;

            delete buf2;
            emit signalLock(false);
            return false;
        }

        oldprogress = progress;
        progress = (i*100)/data->size();
        if (progress > oldprogress) { // Push only if number has increased
            emit transferProgress(progress);
        }

    }
    delete buf2;

    emit transferProgress(100);
    emit signalStatus(tr("Transfer done"));

    emit signalLock(false);

    return true;
}

bool Motolink::verifyFirmware(QByteArray *data)
{
    QDataStream file(data, QIODevice::ReadOnly);
    emit signalLock(true);
    mStopTranfer = false;
    const quint32 buf_size = 508;
    char buf[buf_size];
    quint32 addr, progress, oldprogress;
    QByteArray data_local, data_remote;

    emit signalStatus(tr("Verifying flash"));

    progress = 0;
    for (int i=0; i<data->size(); i+=buf_size)
    {
        if (mStopTranfer)
            break;

        int read = file.readRawData(buf, buf_size);
        data_local.setRawData(buf, read);
        addr = i;

        if (!data_local.size()) break;
        _LOCK_
        int read_size = mBtl->readMem(addr, &data_remote, data_local.size());
        _UNLOCK_
        if (read_size < data_local.size()) { // Read same amount of data as from file.

            emit transferProgress(0);
            emit signalStatus(tr("Verification Failed"));
            qWarning() << tr("Verification Failed");
            emit signalLock(false);
            return false;
        }

        if (data_remote != data_local) {

            emit transferProgress(100);
            emit signalStatus(tr("Verification failed at 0x")+QString::number(addr, 16));

            QString stmp, sbuf;
            for (int b=0;b<data_local.size();b++) {
                stmp.append(QString().sprintf("%02X ", (uchar)data_local.at(b)));
                sbuf.append(QString().sprintf("%02X ", (uchar)data_remote.at(b)));
            }
            qWarning() << tr("Verification failed at 0x")+QString::number(addr, 16) <<
                           "\r\n" << tr("Expecting:") << stmp << "\r\n       " << tr("Got:") << sbuf;
            emit signalLock(false);
            return false;
        }
        oldprogress = progress;
        progress = (i*100)/data->size();
        if (progress > oldprogress) { // Push only if number has increased
            emit transferProgress(progress);
        }
        //emit sendStatus(tr("Verified ")+QString::number(i/1024)+tr(" kilobytes out of ")+QString::number(data->size()/1024));
    }

    emit transferProgress(100);
    emit signalStatus(tr("Verification OK"));
    qDebug() << tr("Verification OK");
    emit signalLock(false);

    return true;
}
