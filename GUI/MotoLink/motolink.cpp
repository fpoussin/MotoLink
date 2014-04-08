#include "motolink.h"
#include "ui_motolink.h"

MotoLink::MotoLink(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MotoLink)
{
    ui->setupUi(this);
}

MotoLink::~MotoLink()
{
    delete ui;
}
