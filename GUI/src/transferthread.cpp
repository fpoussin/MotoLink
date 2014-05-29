/*
This file is part of QSTLink2.

    QSTLink2 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    QSTLink2 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QSTLink2.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "transferthread.h"
#include <QDataStream>

TransferThread::TransferThread(Bootloader *btl, QObject *parent) :
    QThread(parent)
{
    qDebug() << "New Transfer Thread";
    mBtl = btl;
    mStop = false;
}

TransferThread::~TransferThread()
{
    this->exit();
    if (!this->wait(1000)) {
        this->terminate();
        this->wait();
    }
}

void TransferThread::run()
{
    if (mWrite) {
        this->send(&mData);
    }
    if (mVerify) {
        this->verify(&mData);
    }
}
void TransferThread::halt()
{
    mStop = true;
    emit sendStatus(tr("Aborted"));
    emit sendLog(tr("Transfer Aborted"));
}

void TransferThread::setParams(Bootloader *btl, QByteArray *data, bool write, bool verify)
{
    mBtl = btl;
    mData = *data;
    mWrite = write;
    mVerify = verify;
}

void TransferThread::send(QByteArray *data)
{
    QDataStream file(data, QIODevice::ReadOnly);
    emit sendLock(true);
    mStop = false;
    quint32 step_size = 96;
    const quint32 from = 0;
    const quint32 to = data->size();

    quint32 progress, oldprogress;
    qint32 read;
    char *buf2 = new char[step_size];

    qDebug() << tr("File size") << data->size();

    emit sendStatus(tr("Erasing..."));
    if (!mBtl->eraseFlash(data->size())) {

        emit sendStatus(tr("Erase failed"));
        qDebug() << tr("Erase failed");
        emit sendLock(false);
        return;
    }
    else {
        emit sendStatus(tr("Erase OK"));
        qDebug() <<tr( "Erase OK");
    }

    qDebug() << tr("Writing from") << "0x"+QString::number(from, 16) << "to" << "0x"+QString::number(to, 16);

    emit sendStatus(tr("Writing Flash"));

    progress = 0;
    for (int i=0; i<=data->size(); i+=step_size) {

        if (mStop)
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

        int wrote = mBtl->writeFlash(i, &buf, read);

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

void TransferThread::verify(QByteArray *data)
{
    QDataStream file(data, QIODevice::ReadOnly);
    emit sendLock(true);
    mStop = false;
    const quint32 buf_size = 512;
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
        if (mStop)
            break;

        _usleep(50000);

        int read = file.readRawData(buf, buf_size);
        data_local.setRawData(buf, read);
        qDebug() << tr("Read") << data_local.size() << tr("Bytes from disk");
        addr = i;

        if (!data_local.size()) break;
        int read_size = mBtl->readMem(addr, &data_remote, data_local.size());
        if (read_size < data_local.size()) {// Read same amount of data as from file.

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
