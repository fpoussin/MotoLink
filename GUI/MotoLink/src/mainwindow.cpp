#include "mainwindow.h"
#include <ui_main.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setupDefaults();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::Quit()
{
    qApp->quit();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    (void)event;
    this->Quit();
}

void MainWindow::setupDefaults()
{
    ui->statusBar->showMessage("Disconnected");
}

void MainWindow::setupConnections()
{
    QObject::connect(ui->actionAbout_QT, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    QObject::connect(ui->actionQuit, SIGNAL(triggered()), this, SLOT(Quit()));
}
