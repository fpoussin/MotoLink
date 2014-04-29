#include "updatewizard.h"
#include "ui_updatewizard.h"

UpdateWizard::UpdateWizard(QWidget *parent) :
    QWizard(parent),
    mUi(new Ui::UpdateWizard)
{
    mUi->setupUi(this);
    this->addPage(&this->mPage1);
    this->addPage(&this->mPage2);
}

UpdateWizard::~UpdateWizard()
{
    delete mUi;
}

void UpdateWizard::showWizard()
{
    this->show();
}
