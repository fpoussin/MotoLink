#ifndef UPDATEWIZARDPAGE1_H
#define UPDATEWIZARDPAGE1_H

#include <QWizardPage>

namespace Ui {
class UpdateWizardPage1;
}

class UpdateWizardPage1 : public QWizardPage
{
    Q_OBJECT

public:
    explicit UpdateWizardPage1(QWidget *parent = 0);
    ~UpdateWizardPage1();

private:
    Ui::UpdateWizardPage1 *ui;
};

#endif // UPDATEWIZARDPAGE1_H
