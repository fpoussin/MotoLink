#include "updatewizard.h"
#include "ui_updatewizard.h"

UpdateWizard::UpdateWizard(Bootloader *btl, QWidget *parent) :
    QWizard(parent),
    mUi(new Ui::UpdateWizard)
{
    mUi->setupUi(this);
    mBtl = btl;
}

UpdateWizard::~UpdateWizard()
{
    delete mUi;
}

void UpdateWizard::showWizard()
{
    this->show();
}

void UpdateWizard::startUpdate()
{
    mUi->pbProgress->setValue(50);
}

void UpdateWizard::retranslate()
{
    mUi->retranslateUi(this);
}
