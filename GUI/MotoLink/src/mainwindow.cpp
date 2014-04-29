#include "mainwindow.h"
#include <QShortcut>
#include <QSignalMapper>
#include <ui_main.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mUi(new Ui::MainWindow), mUpdateWizard(this)
{
    mUi->setupUi(this);
    this->makeDefaultModel();
    this->setupDefaults();
    this->setupConnections();
    this->setupTabShortcuts();
    this->mHasChanged = false;

    //this->mUpdateWizard = new Ui::UpdateWizard(this);
}

MainWindow::~MainWindow()
{
    delete mUi;
}

void MainWindow::Quit()
{
    if (this->mHasChanged) {

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
    this->mCurrentFile = fileName;
}

void MainWindow::connectToEcu()
{


    this->mUi->tabMain->setEnabled(true);
}

void MainWindow::disconnectFromEcu()
{


    this->mUi->tabMain->setEnabled(false);
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
    this->mUpdateWizard.show();
}

void MainWindow::importHrc()
{
    QString fileName(QFileDialog::getOpenFileName(this,
                       tr("Import HRC File"), "", tr("HRC File (*.E2P)")));
    this->mHrc.openFile(fileName);
}

void MainWindow::exportHrc()
{
    QString fileName(QFileDialog::getSaveFileName(this,
                       tr("Export HRC File"), "", tr("HRC File (*.E2P)")));
    this->mHrc.saveFile(fileName);
}

void MainWindow::setupDefaults(void)
{
    this->mUi->statusBar->showMessage("Disconnected");
    this->mUi->tableFuel->setModel(&this->mDefaultModel);
    this->mUi->tableIgnMap->setModel(&this->mDefaultModel);
    this->mUi->tableAfrMap->setModel(&this->mDefaultModel);
    this->mUi->tableIgnTgt->setModel(&this->mDefaultModel);
    this->mUi->tableAfrTgt->setModel(&this->mDefaultModel);
    this->mUi->tableKnk->setModel(&this->mDefaultModel);
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
}

void MainWindow::setupTabShortcuts()
{
    QSignalMapper *signalMapper = new QSignalMapper(this);

    /* Assign tab shortcuts starting from F5 up to F12 */
    for(int index=0; index < this->mUi->tabMain->count(); ++index)
    {
       if (index > 7) break;
       QShortcut *shortcut = new QShortcut(
                   QKeySequence(QString("F%1").arg(index + 5)), this);

       QObject::connect(shortcut, SIGNAL( activated()),
                         signalMapper, SLOT(map()));
       signalMapper->setMapping(shortcut, index);
    }
    QObject::connect(signalMapper, SIGNAL(mapped(int)),
              this->mUi->tabMain, SLOT(setCurrentIndex(int)));
}

void MainWindow::makeDefaultModel()
{
    this->mNumRow = 11;
    this->mNumCol = 16;

    for (int row = 0; row < this->mNumRow; ++row)
    {
        for (int column = 0; column < this->mNumCol; ++column)
        {
            QStandardItem *item = new QStandardItem(0);
            this->mDefaultModel.setItem(row, column, item);
            this->mDefaultModel.setHeaderData(column, Qt::Horizontal, (1000*column)+1000);
            this->mDefaultModel.setHeaderData(row, Qt::Vertical, QString::number(100-(row*10)) + "%");
            this->mUi->tableFuel->setColumnWidth(column, 15);
            this->mDefaultModel.setData(mDefaultModel.index(row, column), QVariant(QBrush(Qt::darkGreen)), Qt::BackgroundRole);
        }
    }
}
