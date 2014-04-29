#ifndef UPDATEWIZARD_H
#define UPDATEWIZARD_H

#include <QWizard>
#include "updatewizardpage1.h"
#include "updatewizardpage2.h"

namespace Ui {
    class UpdateWizard;
}

class UpdateWizard : public QWizard
{
    Q_OBJECT

public:
    explicit UpdateWizard(QWidget *parent = 0);
    ~UpdateWizard();
    void showWizard(void);

private:
    Ui::UpdateWizard *mUi;
    UpdateWizardPage1 mPage1;
    UpdateWizardPage2 mPage2;

};

#endif // UPDATEWIZARD_H
