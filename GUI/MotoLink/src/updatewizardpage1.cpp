#include "updatewizardpage1.h"
#include "ui_updatewizardpage1.h"

UpdateWizardPage1::UpdateWizardPage1(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::UpdateWizardPage1)
{
    ui->setupUi(this);
}

UpdateWizardPage1::~UpdateWizardPage1()
{
    delete ui;
}
