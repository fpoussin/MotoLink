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
#include <mainwindow.h>
#include <ui_mainwindow.h>
#include <stdlib.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->ui->b_disconnect->setEnabled(false);
    this->ui->gb_top->setEnabled(false);
    this->ui->b_send->setEnabled(false);
    this->ui->b_verify->setEnabled(false);
    this->ui->b_repeat->setEnabled(false);
    this->ui->b_reset->setEnabled(false);
    this->btl = new Bootloader();
    this->tfThread = new transferThread();

    this->lastAction = ACTION_NONE;
    this->btl->moveToThread(&btlThread);
    btlThread.start();

    this->ui->gb_top->setEnabled(true);

    // UI
    QObject::connect(this->ui->b_quit,SIGNAL(clicked()),this,SLOT(Quit()));
    QObject::connect(this->ui->b_qt,SIGNAL(clicked()),qApp,SLOT(aboutQt()));
    QObject::connect(this->ui->b_connect, SIGNAL(clicked()), this, SLOT(Connect()));
    QObject::connect(this->ui->b_disconnect, SIGNAL(clicked()), this, SLOT(Disconnect()));
    QObject::connect(this->ui->b_send, SIGNAL(clicked()), this, SLOT(Send()));
    QObject::connect(this->ui->b_verify, SIGNAL(clicked()), this, SLOT(Verify()));
    QObject::connect(this->ui->b_repeat, SIGNAL(clicked()), this, SLOT(Repeat()));
    QObject::connect(this->ui->b_reset, SIGNAL(clicked()), this, SLOT(ResetMCU()));

    // Thread
    QObject::connect(this->tfThread, SIGNAL(sendProgress(quint32)), this, SLOT(updateProgress(quint32)));
    QObject::connect(this->tfThread, SIGNAL(sendStatus(QString)), this, SLOT(updateStatus(QString)));
    QObject::connect(this->tfThread, SIGNAL(sendLock(bool)), this, SLOT(lockUI(bool)));
    QObject::connect(this->ui->b_stop, SIGNAL(clicked()), this->tfThread, SLOT(halt()));
    QObject::connect(this->tfThread, SIGNAL(sendLog(QString)), this, SLOT(log(QString)));

    QObject::connect(this, SIGNAL(doConnect()), this->btl, SLOT(connect()));
    QObject::connect(this->btl, SIGNAL(connectionResult(bool)), this, SLOT(connectSlot(bool)));
}

MainWindow::~MainWindow()
{
    this->tfThread->exit();
    delete tfThread;
    delete btl;
    delete ui;
}

void MainWindow::showHelp()
{

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    (void)event;
    this->Quit();
}

void MainWindow::connectSlot(bool success)
{
    PrintFuncName();
    switch (success) {
    case false:
        this->ui->b_connect->setEnabled(true);
        this->log("Device not found or unable to access it.");
#if defined(QWINUSB) && defined(WIN32)
        this->log("Did you install the drivers ?");
#elif !defined(WIN32)
        this->log("Did you install the udev rules ?");
#else
        this->log("Did you install the libusb-win32 driver ?");
#endif
        return;
    case true:
        this->log("Device found!");

        this->ui->b_connect->setEnabled(false);
        this->ui->b_disconnect->setEnabled(true);
        if (true) {
            this->ui->gb_bottom->setEnabled(true);
            this->ui->b_send->setEnabled(true);
            this->ui->b_verify->setEnabled(true);
            this->ui->b_repeat->setEnabled(true);
            this->ui->b_reset->setEnabled(true);
            return;
        }
        else
            return;
    }
}

bool MainWindow::Connect()
{
    PrintFuncName();
    this->log("Searching Device...");

    this->ui->b_connect->setEnabled(false);
    emit doConnect();

    return false;
}

void MainWindow::Disconnect()
{
    this->log("Disconnecting...");
    this->btl->disconnect();
    this->log("Disconnected.");
    qInformal() << "Disconnected.";
    this->ui->b_disconnect->setEnabled(false);
    this->ui->b_connect->setEnabled(true);
    this->ui->gb_bottom->setEnabled(false);
    this->ui->b_send->setEnabled(false);
    this->ui->b_verify->setEnabled(false);
    this->ui->b_repeat->setEnabled(false);
    this->ui->b_reset->setEnabled(false);
}

void MainWindow::log(const QString &s)
{
    this->ui->t_log->appendPlainText(s);
}

void MainWindow::lockUI(bool enabled)
{
    this->ui->gb_top->setEnabled(!enabled);
    this->ui->gb_bottom->setEnabled(!enabled);
}

void MainWindow::updateProgress(quint32 p)
{
    this->ui->pgb_transfer->setValue(p);
}

void MainWindow::updateStatus(const QString &s)
{
    this->ui->l_progress->setText(s);
}

void MainWindow::Send()
{
    this->filename.clear();
    this->filename = QFileDialog::getOpenFileName(this, "Open file", "", "Binary Files (*.bin)");
    if (!this->filename.isNull()) {
        QFile file(this->filename);
        if (!file.open(QIODevice::ReadOnly)) {
            qCritical("Could not open the file.");
            return;
        }
        file.close();
        this->log("Size: "+QString::number(file.size()/1024)+"KB");

        if (file.size() > 224*1024) {
            if(QMessageBox::question(this, "Flash size exceeded", "The file is bigger than the flash size!\n\nThe flash memory will be erased and the new file programmed, continue?", QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes){
                return;
            }
        }
        else {
            if(QMessageBox::question(this, "Confirm", "The flash memory will be erased and the new file programmed, continue?", QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes){
                return;
            }
        }
        file.close();

        this->Send(this->filename);
        this->lastAction = ACTION_SEND;
    }
}

void MainWindow::Send(const QString &path)
{
    qDebug("Writing flash");
    this->log("Sending "+path);
    this->ui->pgb_transfer->setValue(0);
    this->ui->l_progress->setText("Starting transfer...");

    // Transfer thread
    this->tfThread->setParams(this->btl, path, true, false);
    this->tfThread->start();
}

void MainWindow::Verify()
{
    qDebug("Verify flash");
    this->filename.clear();
    this->filename = QFileDialog::getOpenFileName(this, "Open file", "", "Binary Files (*.bin)");
    if (!this->filename.isNull()) {
        QFile file(this->filename);
        if (!file.open(QIODevice::ReadOnly)) {
            qCritical("Could not open the file.");
            return;
        }
        file.close();
        this->Verify(this->filename);
        this->lastAction = ACTION_VERIFY;
    }
}

void MainWindow::Repeat()
{
    switch (this->lastAction) {

        case ACTION_SEND:
            this->Send(this->filename);
            break;
        case ACTION_VERIFY:
            this->Verify(this->filename);
            break;
        case ACTION_NONE:
            this->log("Nothing to repeat.");
            break;
        default:
            break;
    }
}

void MainWindow::Verify(const QString &path)
{
    this->log("Verifying "+path);
    this->ui->pgb_transfer->setValue(0);
    this->ui->l_progress->setText("Starting Verification...");

    // Transfer thread
    this->tfThread->setParams(this->btl, path, false, true);
    this->tfThread->start();
}

void MainWindow::ResetMCU()
{
    this->log("Reseting MCU...");
    this->btl->reset();
    usleep(100000);
}


void MainWindow::Quit()
{
    this->hide();
    if (this->btl->isConnected())
        this->Disconnect();
    qApp->quit();
}


