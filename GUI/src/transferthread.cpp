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

transferThread::transferThread(QObject *parent) :
    QThread(parent)
{
    qDebug() << "New Transfer Thread";
    mStop = false;
}

transferThread::~transferThread()
{
    this->exit();
    if (!this->wait(1000)) {
        this->terminate();
        this->wait();
    }
}

void transferThread::run()
{
    if (mWrite) {
            this->send(mFilename);
        if (mVerify)
            this->verify(mFilename);
    }
    else {
        this->verify(mFilename);
    }
}
void transferThread::halt()
{
    mStop = true;
    emit sendStatus(tr("Aborted"));
    emit sendLog(tr("Transfer Aborted"));
}

void transferThread::setParams(Bootloader *btl, QString filename, bool write, bool verify)
{
    mBtl = btl;
    mFilename = filename;
    mWrite = write;
    mVerify = verify;
}

void transferThread::send(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << tr("Could not open the file.");
        return;
    }
    emit sendLock(true);
    mStop = false;
    quint32 step_size = 116;
    const quint32 from = 0;
    const quint32 to = file.size();

    quint32 progress, oldprogress;
    qint32 read;
    char *buf2 = new char[step_size];

    qDebug() << tr("File size") << file.size();

    if (!mBtl->eraseFlash(file.size())) {

        emit sendStatus(tr("Erase failed"));
        emit sendLog(tr("Erase failed"));
        qDebug() << tr("Erase failed");
        emit sendLock(false);
        return;
    }
    else {
        emit sendStatus(tr("Erase OK"));
        emit sendLog(tr("Erase OK"));
        qDebug() <<tr( "Erase OK");
    }

    qDebug() << tr("Writing from") << "0x"+QString::number(from, 16) << "to" << "0x"+QString::number(to, 16);

    emit sendStatus(tr("Transfering"));
    emit sendLog(tr("Transfering"));

    progress = 0;
    for (int i=0; i<=file.size(); i+=step_size) {

        if (mStop)
            break;

        if (file.atEnd()) {
            qDebug() << tr("End Of File");
            break;
        }

        memset(buf2, 0, step_size);
        if ((read = file.read(buf2, step_size)) <= 0)
            break;
        qDebug() << tr("Read") << read << tr("Bytes from disk");
        QByteArray buf(buf2, read);

        if (mBtl->writeFlash(i, &buf, step_size) < read){
            emit sendStatus(tr("Transfer failed"));
            emit sendLog(tr("Transfer failed"));
            break;
        }

        oldprogress = progress;
        progress = (i*100)/file.size();
        if (progress > oldprogress) { // Push only if number has increased
            emit sendProgress(progress);
            qDebug() << tr("Progress:") << QString::number(progress)+"%";
        }

    }
    emit sendLoaderStatus(tr("Idle"));
    file.close();
    delete buf2;

    emit sendProgress(100);
    emit sendStatus(tr("Transfer done"));
    emit sendLog(tr("Transfer done"));
    qDebug() << tr("Transfer done");

    emit sendLock(false);
}

void transferThread::verify(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << tr("Could not open the file.");
        return;
    }
    emit sendLock(true);
    mStop = false;
    const quint32 buf_size = 512;
    const quint32 from = 0;
    const quint32 to = file.size();
    qDebug() << tr("Reading from") << QString::number(from, 16) << tr("to") << QString::number(to, 16);
    quint32 addr, progress, oldprogress;
    QByteArray data_local, data_remote;

    progress = 0;
    for (quint32 i=0; i<file.size(); i+=buf_size)
    {
        if (mStop)
            break;

        _usleep(50000);

        data_local = file.read(buf_size);
        qDebug() << tr("Read") << data_local.size() << tr("Bytes from disk");
        addr = i;

        if (!data_local.size()) break;
        int read_size = mBtl->readMem(addr, &data_remote, data_local.size());
        if (read_size < data_local.size()) {// Read same amount of data as from file.

            file.close();
            emit sendProgress(0);
            emit sendStatus(tr("Verification Failed"));
            qWarning() << tr("Verification Failed");
            qDebug() << tr("read") << read_size << tr("valid") << data_local.size();
            emit sendLock(false);
            return;
        }

        if (data_remote != data_local) {

            file.close();
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
        progress = (i*100)/file.size();
        if (progress > oldprogress) { // Push only if number has increased
            emit sendProgress(progress);
            qDebug() << tr("Progress:") << QString::number(progress)+"%";
        }
        emit sendStatus(tr("Verified ")+QString::number(i/1024)+tr(" kilobytes out of ")+QString::number(file.size()/1024));
    }

    file.close();
    emit sendProgress(100);
    emit sendStatus(tr("Verification OK"));
    qDebug() << tr("Verification OK");
    emit sendLock(false);
}
