#ifndef UPDATEWIZARD_H
#define UPDATEWIZARD_H

#include <QWizard>

#include "bootloader.h"
#include "motolink.h"

namespace Ui {
    class UpdateWizard;
}

class UpdateWizard : public QWizard
{
    Q_OBJECT

public:
    explicit UpdateWizard(Bootloader *btl, QWidget *parent = 0);
    ~UpdateWizard();
    void showWizard(void);

public slots:
    void startUpdate(void);
    void retranslate(void);

private:
    Ui::UpdateWizard *mUi;
    Bootloader *mBtl;
    Motolink *mMtl;
};

#endif // UPDATEWIZARD_H
