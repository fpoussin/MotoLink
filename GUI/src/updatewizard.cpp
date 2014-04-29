#include "updatewizard.h"
#include "ui_updatewizard.h"

UpdateWizard::UpdateWizard(QWidget *parent) :
    QWizard(parent),
    mUi(new Ui::UpdateWizard)
{
    mUi->setupUi(this);
}

UpdateWizard::~UpdateWizard()
{
    delete mUi;
}

void UpdateWizard::showWizard()
{
    this->show();
}
