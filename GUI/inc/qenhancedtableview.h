#ifndef QENHANCEDTABLEVIEW_H
#define QENHANCEDTABLEVIEW_H

#include <QDialog>
#include <QTableView>
#include <QHeaderView>
#include <QItemSelectionModel>
#include "ui_headeredit.h"
#include "ui_celledit.h"

namespace Ui {
    class HeaderEdit;
    class CellEdit;
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
    void cellCleared(uint id, int row, int col);

public slots:
    void retranslate(void);
    void showContextMenu(const QPoint &pos);

private slots:
    void clickedVerticalHeader(int section);
    void clickedHorizontalHeader(int section);
    void applyHeaderChanges(void);
    void setTabFocus(void);
    void applyCellChanges(void);

private:
    void setupConnections(void);
    void setEditBoundaries(int section, Qt::Orientation orientation);

    Ui::HeaderEdit *mHeaderEditUi;
    Ui::CellEdit *mCellEditUi;
    QDialog *mHeaderEditDialog;
    QDialog *mCellEditDialog;
    int mLastSection;
    Qt::Orientation mLastOrientation;
    bool mMenuReadOnly;

};

#endif // QENHANCEDTABLEVIEW_H
