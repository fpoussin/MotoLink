#ifndef QENHANCEDTABLEVIEW_H
#define QENHANCEDTABLEVIEW_H

#include <QDialog>
#include <QTableView>
#include <QHeaderView>
#include <QItemSelectionModel>
#include "ui_headeredit.h"

namespace Ui {
    class HeaderEdit;
}

class QEnhancedTableView : public QTableView
{
    Q_OBJECT
public:
    explicit QEnhancedTableView(QWidget *parent = 0);
    void setModel(QAbstractItemModel * model);
    void setMenuReadOnly(bool enabled);

signals:
    void modelUpdated(QWidget* widget);

public slots:
    void retranslate(void);
    void showContextMenu(const QPoint &pos);

private slots:
    void clickedVerticalHeader(int section);
    void clickedHorizontalHeader(int section);
    void applyChanges(void);
    void setTabFocus(void);

private:
    void setupConnections(void);
    void setEditBoundaries(int section, Qt::Orientation orientation);

    QDialog *mHeaderEditDialog;
    Ui::HeaderEdit *mHeaderEditUi;
    int mLastSection;
    Qt::Orientation mLastOrientation;
    bool mMenuReadOnly;

};

#endif // QENHANCEDTABLEVIEW_H
