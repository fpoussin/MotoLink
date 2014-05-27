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

private slots:
    void pageUpdated(int page);

private:
    void setupConnections(void);
    void loadFirmareData(void);
    void connectBtl(void);
    Ui::UpdateWizard *mUi;
    Bootloader *mBtl;
    Motolink *mMtl;
    QByteArray mFwData;
    QString mCurVersion;
    QString mNewVersion;
};

#endif // UPDATEWIZARD_H
