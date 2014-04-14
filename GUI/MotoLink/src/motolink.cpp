#include "motolink.h"
#include "ui_motolink.h"

MotoLink::MotoLink(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MotoLink)
{
    ui->setupUi(this);
    this->setupDefaults();
}

MotoLink::~MotoLink()
{
    delete ui;
}

void MotoLink::setupDefaults()
{
    ui->statusBar->showMessage("Disconnected");
}
