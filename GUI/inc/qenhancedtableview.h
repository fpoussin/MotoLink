#ifndef QENHANCEDTABLEVIEW_H
#define QENHANCEDTABLEVIEW_H

#include <QDialog>
#include <QTableView>
#include <QHeaderView>
#include <QPaintEvent>
#include "ui_headeredit.h"

namespace Ui {
    class HeaderEdit;
}

class QEnhancedTableView : public QTableView
{
    Q_OBJECT
public:
    explicit QEnhancedTableView(QWidget *parent = 0);

signals:

public slots:

private slots:
    void clickedVerticalHeader(int section);
    void clickedHorizontalHeader(int section);
    void applyChanges(void);

private:
    void setupConnections(void);
    void setEditBoundaries(int section, Qt::Orientation orientation);

    QDialog *mHeaderEditDialog;
    Ui::HeaderEdit *mHeaderEditUi;
    int mLastSection;
    Qt::Orientation mLastOrientation;
};

#endif // QENHANCEDTABLEVIEW_H
