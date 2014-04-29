#ifndef UPDATEWIZARDPAGE2_H
#define UPDATEWIZARDPAGE2_H

#include <QWizardPage>

namespace Ui {
class UpdateWizardPage2;
}

class UpdateWizardPage2 : public QWizardPage
{
    Q_OBJECT

public:
    explicit UpdateWizardPage2(QWidget *parent = 0);
    ~UpdateWizardPage2();

private:
    Ui::UpdateWizardPage2 *ui;
};

#endif // UPDATEWIZARDPAGE2_H
