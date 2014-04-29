#include "mainwindow.h"
#include <QShortcut>
#include <QSignalMapper>
#include <ui_main.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mUi(new Ui::MainWindow),
    mUsb(),
    mMtl(&mUsb),
    mBtl(&mUsb),
    mUpdateWizard(&mBtl, this)
{
    mUi->setupUi(this);
    makeDefaultModel();
    this->setupDefaults();
    this->setupConnections();
    this->setupTabShortcuts();
    mHasChanged = false;
}

MainWindow::~MainWindow()
{
    delete mUi;
}

void MainWindow::Quit()
{
    if (mHasChanged) {

        int ret = QMessageBox::question(this, tr("Confirm"),
                    tr("Unsaved changes!\nSave file before closing?"),
                    QMessageBox::Save | QMessageBox::Discard
                    | QMessageBox::Cancel,
                    QMessageBox::Cancel);

        switch (ret) {
          case QMessageBox::Save:
              this->saveFile();
              break;
          case QMessageBox::Discard:
              break;
          case QMessageBox::Cancel:
              return;
        }
    }
    qApp->quit();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    this->Quit();
}

void MainWindow::openFile(void)
{
    QString fileName(QFileDialog::getOpenFileName(this,
                       tr("Open Tune File"), "", tr("Tune Files (*.xml)")));


}

void MainWindow::saveFile(void)
{
    if (mCurrentFile.length() == 0)
    {
        this->saveFileAs();
    }
}

void MainWindow::saveFileAs(void)
{
    QString fileName(QFileDialog::getSaveFileName(this,
                       tr("Save Tune File"), "", tr("Tune Files (*.xml)")));

    if (fileName.length() == 0)
    {
        return;
    }
    mCurrentFile = fileName;
}

void MainWindow::connectToEcu()
{

    mUi->tabMain->setEnabled(true);
}

void MainWindow::disconnectFromEcu()
{


    mUi->tabMain->setEnabled(false);
}

void MainWindow::showAbout()
{
    QMessageBox::information(this, "About Motolink",
       "<strong>Version: " __MTL_VER__ "</strong><br/><br/>"
       "Motolink is a smart interface designed for Honda HRC ECUs.<br/><br/>"
       "You can find more information "
       "<a href=\"https://github.com/mobyfab/MotoLink\">here.</a>");
}

void MainWindow::showUpdateDialog()
{
    mUpdateWizard.showWizard();
}

void MainWindow::importHrc()
{
    QString fileName(QFileDialog::getOpenFileName(this,
                       tr("Import HRC File"), "", tr("HRC File (*.E2P)")));
    mHrc.openFile(fileName);
}

void MainWindow::exportHrc()
{
    QString fileName(QFileDialog::getSaveFileName(this,
                       tr("Export HRC File"), "", tr("HRC File (*.E2P)")));
    mHrc.saveFile(fileName);
}

void MainWindow::setupDefaults(void)
{
    mUi->statusBar->showMessage("Disconnected");
    mUi->tableFuel->setModel(&mDefaultModel);
    mUi->tableIgnMap->setModel(&mDefaultModel);
    mUi->tableAfrMap->setModel(&mDefaultModel);
    mUi->tableIgnTgt->setModel(&mDefaultModel);
    mUi->tableAfrTgt->setModel(&mDefaultModel);
    mUi->tableKnk->setModel(&mDefaultModel);
}

void MainWindow::setupConnections(void)
{
    QObject::connect(mUi->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    QObject::connect(mUi->actionAbout_QT, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    QObject::connect(mUi->actionQuit, SIGNAL(triggered()), this, SLOT(Quit()));
    QObject::connect(mUi->actionOpen, SIGNAL(triggered()), this, SLOT(openFile()));
    QObject::connect(mUi->actionSave, SIGNAL(triggered()), this, SLOT(saveFile()));
    QObject::connect(mUi->actionSave_As, SIGNAL(triggered()), this, SLOT(saveFileAs()));
    QObject::connect(mUi->actionConnect, SIGNAL(triggered()), this, SLOT(connectToEcu()));
    QObject::connect(mUi->actionDisconnect, SIGNAL(triggered()), this, SLOT(disconnectFromEcu()));
    QObject::connect(mUi->actionUpdate, SIGNAL(triggered()), this, SLOT(showUpdateDialog()));
    QObject::connect(mUi->actionImport, SIGNAL(triggered()), this, SLOT(importHrc()));
    QObject::connect(mUi->actionExport, SIGNAL(triggered()), this, SLOT(exportHrc()));


    /*
     * Signals from old bootloader GUI, just for reference, will remove later
    // Thread
    QObject::connect(this->tfThread, SIGNAL(sendProgress(quint32)), this, SLOT(updateProgress(quint32)));
    QObject::connect(this->tfThread, SIGNAL(sendStatus(QString)), this, SLOT(updateStatus(QString)));
    QObject::connect(this->tfThread, SIGNAL(sendLock(bool)), this, SLOT(lockUI(bool)));
    QObject::connect(this->ui->b_stop, SIGNAL(clicked()), this->tfThread, SLOT(halt()));
    QObject::connect(this->tfThread, SIGNAL(sendLog(QString)), this, SLOT(log(QString)));

    QObject::connect(this, SIGNAL(doConnect()), this->btl, SLOT(connect()));
    QObject::connect(this->btl, SIGNAL(connectionResult(bool)), this, SLOT(connectSlot(bool)));
    QObject::connect(&this->connectDialog, SIGNAL(canceled()), this, SLOT(connectAbortSlot()));
    QObject::connect(this->btl, SIGNAL(timeElapsed(int)), &this->connectDialog, SLOT(setValue(int)));
    */


}

void MainWindow::setupTabShortcuts()
{
    QSignalMapper *signalMapper = new QSignalMapper(this);

    /* Assign tab shortcuts starting from F5 up to F12 */
    for(int index=0; index < mUi->tabMain->count(); ++index)
    {
       if (index > 7) break;
       QShortcut *shortcut = new QShortcut(
                   QKeySequence(QString("F%1").arg(index + 5)), this);

       QObject::connect(shortcut, SIGNAL( activated()),
                         signalMapper, SLOT(map()));
       signalMapper->setMapping(shortcut, index);
    }
    QObject::connect(signalMapper, SIGNAL(mapped(int)),
              mUi->tabMain, SLOT(setCurrentIndex(int)));
}

void MainWindow::makeDefaultModel()
{
    mNumRow = 11;
    mNumCol = 16;

    for (int row = 0; row < mNumRow; ++row)
    {
        for (int column = 0; column < mNumCol; ++column)
        {
            QStandardItem *item = new QStandardItem(0);
            mDefaultModel.setItem(row, column, item);
            mDefaultModel.setHeaderData(column, Qt::Horizontal, (1000*column)+1000);
            mDefaultModel.setHeaderData(row, Qt::Vertical, QString::number(100-(row*10)) + "%");
            mUi->tableFuel->setColumnWidth(column, 15);
            mDefaultModel.setData(mDefaultModel.index(row, column), QVariant(QBrush(Qt::darkGreen)), Qt::BackgroundRole);
        }
    }
}
