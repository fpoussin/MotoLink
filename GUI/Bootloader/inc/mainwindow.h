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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QFileDialog>
#include <QFile>
#include <QByteArray>
#include <transferthread.h>
#include <compat.h>
#include <QMessageBox>
#include <bootloader.h>
#include <QThread>
#include <QTimer>
#include <QProgressDialog>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    enum {
        ACTION_NONE = 0,
        ACTION_SEND = 1,
        ACTION_VERIFY = 2
    };

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    transferThread *tfThread;

public slots:
    bool Connect();
    void Disconnect();
    void updateProgress(quint32 p);
    void updateStatus(const QString &s);
    void Send(const QString &path);
    void Verify(const QString &path);
    void showHelp();
    void closeEvent(QCloseEvent *event);

private:
    Ui::MainWindow *ui;
    Bootloader *btl;
    QString filename;
    quint32 lastAction;
    QThread btlThread;
    QProgressDialog connectDialog;

private slots:
    void connectSlot(bool success);
    void connectAbortSlot(void);
    void lockUI(bool enabled);
    void log(const QString &s);
    void Send();
    void Verify();
    void Repeat();
    void ResetMCU();
    void Quit();

signals:
    void doConnect();

};

#endif // MAINWINDOW_H
