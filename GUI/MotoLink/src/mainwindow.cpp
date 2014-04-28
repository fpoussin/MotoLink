#include "mainwindow.h"
#include <QShortcut>
#include <QSignalMapper>
#include <ui_main.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->makeDefaultModel();
    this->setupDefaults();
    this->setupConnections();
    this->setupTabShortcuts();
    this->mHasChanged = false;
}

MainWindow::~MainWindow()
{
    delete ui;
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
    QString fileName = QFileDialog::getOpenFileName(this,
                       tr("Open Tune File"), "", tr("Tune Files (*.xml)"));
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
    QString fileName = QFileDialog::getSaveFileName(this,
                       tr("Save Tune File"), "", tr("Tune Files (*.xml)"));

    if (fileName.length() == 0)
    {
        return;
    }
    this->mCurrentFile = fileName;
}

void MainWindow::connectToEcu()
{


    this->ui->tabMain->setEnabled(true);
}

void MainWindow::disconnectFromEcu()
{


    this->ui->tabMain->setEnabled(false);
}

void MainWindow::showAbout()
{
    QMessageBox::about(this, "About Motolink",
                       "<strong>Version: " __MTL_VER__ "</strong><br/><br/>"
                       "Motolink is a smart cable designed for HRC ECUs.<br/><br/>"
                       "You can find more information <a href=\"https://github.com/mobyfab/MotoLink\">here.</a>");
}

void MainWindow::setupDefaults(void)
{
    this->ui->statusBar->showMessage("Disconnected");
    this->ui->tableFuel->setModel(&this->mDefaultModel);
    this->ui->tableIgnMap->setModel(&this->mDefaultModel);
    this->ui->tableAfrMap->setModel(&this->mDefaultModel);
    this->ui->tableIgnTgt->setModel(&this->mDefaultModel);
    this->ui->tableAfrTgt->setModel(&this->mDefaultModel);
    this->ui->tableKnk->setModel(&this->mDefaultModel);
}

void MainWindow::setupConnections(void)
{
    QObject::connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    QObject::connect(ui->actionAbout_QT, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    QObject::connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(Quit()));
    QObject::connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(openFile()));
    QObject::connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(saveFile()));
    QObject::connect(ui->actionSave_As, SIGNAL(triggered()), this, SLOT(saveFileAs()));
    QObject::connect(ui->actionConnect, SIGNAL(triggered()), this, SLOT(connectToEcu()));
    QObject::connect(ui->actionDisconnect, SIGNAL(triggered()), this, SLOT(disconnectFromEcu()));
}

void MainWindow::setupTabShortcuts()
{
    QSignalMapper *signalMapper = new QSignalMapper(this);

    for( int index=0; index < this->ui->tabMain->count(); ++index ){
           QShortcut *shortcut = new QShortcut( QKeySequence(QString("F%1").arg( index +5 ) ), this );

           QObject::connect( shortcut, SIGNAL(activated() ), signalMapper, SLOT( map() ) );
           signalMapper->setMapping( shortcut, index );
        }
        QObject::connect( signalMapper, SIGNAL(mapped( int )),
                  this->ui->tabMain, SLOT(setCurrentIndex( int )) );
}

void MainWindow::makeDefaultModel()
{
    this->mNumRow = 11;
    this->mNumCol = 16;

    for (int row = 0; row < this->mNumRow; ++row)  {
        for (int column = 0; column < this->mNumCol; ++column)  {
            QStandardItem *item = new QStandardItem(0);
            this->mDefaultModel.setItem(row, column, item);
            this->mDefaultModel.setHeaderData(column, Qt::Horizontal, (1000*column)+1000);
            this->mDefaultModel.setHeaderData(row, Qt::Vertical, QString::number(100-(row*10)) + "%");
            this->ui->tableFuel->setColumnWidth(column, 15);
            this->mDefaultModel.setData(mDefaultModel.index(row, column), QVariant(QBrush(Qt::darkGreen)), Qt::BackgroundRole);
        }
    }
}
