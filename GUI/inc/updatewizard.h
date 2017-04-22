#ifndef UPDATEWIZARD_H
#define UPDATEWIZARD_H

#include <QWizard>
#include <qabstractbutton.h>
#include <QFileDialog>

#include "bootloader.h"
#include "motolink.h"

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
    void openCustomFw(void);
    QString getFwVersion(void);

signals:
    void sendDisconnect(void);
    void startTransfer(QByteArray *data);
    void send(QByteArray *data);
    void verify(QByteArray *data);
    void sendStatus(QString str);

private slots:
    void pageUpdated(int page);
    void updateStatus(QString text);
    void enableButtons(void);
    void disableButtons(void);
    void updateDone(void);
    void loadDefaultFirmareData(void);

private:
    void setupConnections(void);
    void startFwUpdate(void);
    Ui::UpdateWizard *mUi;
    Motolink * const mMtl;
    QByteArray mFwData;
    QString mCurVersion;
    QString mNewVersion;
    QString msError;
};

#endif // UPDATEWIZARD_H
