#include "updatewizardpage2.h"
#include "ui_updatewizardpage2.h"

UpdateWizardPage2::UpdateWizardPage2(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::UpdateWizardPage2)
{
    ui->setupUi(this);
}

UpdateWizardPage2::~UpdateWizardPage2()
{
    delete ui;
}
