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
    this->m_stop = false;
}

void transferThread::run()
{
    if (this->m_write) {
            this->send(this->m_filename);
        if (this->m_verify)
            this->verify(this->m_filename);
    }
    else {
        this->verify(this->m_filename);
    }
}
void transferThread::halt()
{
    this->m_stop = true;
    emit sendStatus("Aborted");
    emit sendLog("Transfer Aborted");
}

void transferThread::setParams(Bootloader *btl, QString filename, bool write, bool verify)
{
    this->m_btl = btl;
    this->m_filename = filename;
    this->m_write = write;
    this->m_verify = verify;
}

void transferThread::send(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical("Could not open the file.");
        return;
    }
    emit sendLock(true);
    this->m_stop = false;
    quint32 step_size = 116;
    const quint32 from = 0;
    const quint32 to = file.size();

    quint32 progress, oldprogress;
    qint32 read;
    char *buf2 = new char[step_size];

    qDebug() << "File size" << file.size();

    if (!this->m_btl->eraseFlash(file.size())) {

        emit sendStatus("Erase failed");
        emit sendLog("Erase failed");
        qDebug() << "Erase failed";
        emit sendLock(false);
        return;
    }
    else {
        emit sendStatus("Erase OK");
        emit sendLog("Erase OK");
        qDebug() << "Erase OK";
    }

    qInformal() << "Writing from" << "0x"+QString::number(from, 16) << "to" << "0x"+QString::number(to, 16);

    emit sendStatus("Transfering");
    emit sendLog("Transfering");

    progress = 0;
    for (int i=0; i<=file.size(); i+=step_size) {

        if (this->m_stop)
            break;

        if (file.atEnd()) {
            qDebug() << "End Of File";
            break;
        }

        memset(buf2, 0, step_size);
        if ((read = file.read(buf2, step_size)) <= 0)
            break;
        qDebug() << "Read" << read << "Bytes from disk";
        QByteArray buf(buf2, read);

        if (this->m_btl->writeFlash(i, &buf, step_size) < read){
            emit sendStatus("Transfer failed");
            emit sendLog("Transfer failed");
            break;
        }

        oldprogress = progress;
        progress = (i*100)/file.size();
        if (progress > oldprogress) { // Push only if number has increased
            emit sendProgress(progress);
            qInformal() << "Progress:"<< QString::number(progress)+"%";
        }

    }
    emit sendLoaderStatus("Idle");
    file.close();
    delete buf2;

    emit sendProgress(100);
    emit sendStatus("Transfer done");
    emit sendLog("Transfer done");
    qInformal() << "Transfer done";

    emit sendLock(false);
}

void transferThread::verify(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical("Could not open the file.");
        return;
    }
    emit sendLock(true);
    this->m_stop = false;
    const quint32 buf_size = 512;
    const quint32 from = 0;
    const quint32 to = file.size();
    qInformal() << "Reading from" << QString::number(from, 16) << "to" << QString::number(to, 16);
    quint32 addr, progress, oldprogress;
    QByteArray data_local, data_remote;

    progress = 0;
    for (quint32 i=0; i<file.size(); i+=buf_size)
    {
        if (this->m_stop)
            break;

        usleep(50000);

        data_local = file.read(buf_size);
        qDebug() << "Read" << data_local.size() << "Bytes from disk";
        addr = i;

        if (!data_local.size()) break;
        int read_size = this->m_btl->readMem(addr, &data_remote, data_local.size());
        if (read_size < data_local.size()) {// Read same amount of data as from file.

            file.close();
            emit sendProgress(0);
            emit sendStatus("Verification Failed");
            qInformal() << "Verification Failed";
            qInformal() << "read" << read_size << "valid" << data_local.size();
            emit sendLock(false);
            return;
        }

        if (data_remote != data_local) {

            file.close();
            emit sendProgress(100);
            emit sendStatus("Verification failed at 0x"+QString::number(addr, 16));

            QString stmp, sbuf;
            for (int b=0;b<data_local.size();b++) {
                stmp.append(QString().sprintf("%02X ", (uchar)data_local.at(b)));
                sbuf.append(QString().sprintf("%02X ", (uchar)data_remote.at(b)));
            }
            qCritical() << "Verification failed at 0x"+QString::number(addr, 16) <<
                           "\r\n Expecting:" << stmp << "\r\n       Got:" << sbuf;
            emit sendLock(false);
            return;
        }
        oldprogress = progress;
        progress = (i*100)/file.size();
        if (progress > oldprogress) { // Push only if number has increased
            emit sendProgress(progress);
            qInformal() << "Progress:"<< QString::number(progress)+"%";
        }
        emit sendStatus("Verified "+QString::number(i/1024)+" kilobytes out of "+QString::number(file.size()/1024));
    }

    file.close();
    emit sendProgress(100);
    emit sendStatus("Verification OK");
    qInformal() << "Verification OK";
    emit sendLock(false);
}
