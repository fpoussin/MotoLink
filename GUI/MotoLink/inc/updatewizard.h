#ifndef UPDATEWIZARD_H
#define UPDATEWIZARD_H

#include <QWizard>

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
};

#endif // UPDATEWIZARD_H
