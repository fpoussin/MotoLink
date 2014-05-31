#ifndef UPDATEWIZARD_H
#define UPDATEWIZARD_H

#include <QWizard>

#include "bootloader.h"
#include "motolink.h"
#include "transferthread.h"

namespace Ui {
    class UpdateWizard;
}

class UpdateWizard : public QWizard
{
    Q_OBJECT

public:
    explicit UpdateWizard(Motolink * const mtl, QWidget *parent = 0);
    ~UpdateWizard();
    void showWizard(void);

public slots:
    void startUpdate(void);
    void retranslate(void);

signals:
    void startTransfer(void);
    void send(QByteArray *data);
    void verify(QByteArray *data);

private slots:
    void pageUpdated(int page);
    void updateStatus(QString text);
    void disableButtons(bool disable);

private:
    void setupConnections(void);
    void loadFirmareData(void);
    void startFwUpdate(void);
    Ui::UpdateWizard *mUi;
    Motolink * const mMtl;
    QByteArray mFwData;
    QString mCurVersion;
    QString mNewVersion;
};

#endif // UPDATEWIZARD_H
