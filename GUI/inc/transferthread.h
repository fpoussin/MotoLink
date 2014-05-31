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
#ifndef TRANSFERTHREAD_H
#define TRANSFERTHREAD_H

#include <QThread>
#include <QDebug>
#include <QString>
#include <QFile>
#include <compat.h>
#include <bootloader.h>

class TransferThread : public QThread
{
    Q_OBJECT
public:
    explicit TransferThread(Bootloader * const btl, QObject *parent = 0);
    ~TransferThread();
    void run();
    void setParams(QByteArray * const data, bool write, bool verify);

signals:
    void sendProgress(int p);
    void sendStatus(const QString &s);
    void sendError(const QString &s);
    void sendLock(bool enabled);
    void sendLog(const QString &s);

public slots:
    void halt();
    void send(QByteArray *data);
    void verify(QByteArray *data);

private slots:

private:
    QByteArray mData;
    Bootloader * const mBtl;
    bool mWrite;
    bool mStop;
    bool mErase;
    bool mVerify;
};

#endif // TRANSFERTHREAD_H
