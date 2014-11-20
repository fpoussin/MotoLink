#include "qenhancedtableview.h"
#include <QDebug>
#include <QMessageBox>
#include <QPainter>
#include <QStaticText>
#include <QMenu>
#include "tablemodel.h"

QEnhancedTableView::QEnhancedTableView(QWidget *parent) :
    QTableView(parent),
    mHeaderEditUi(new Ui::HeaderEdit),
    mHeaderEditDialog(new QDialog),
    mCellEditUi(new Ui::CellEdit),
    mCellEditDialog(new QDialog)
{
    mHeaderEditUi->setupUi(mHeaderEditDialog);
    mHeaderEditDialog->setParent(this);
    mHeaderEditDialog->setWindowFlags(Qt::Dialog
                                      | Qt::FramelessWindowHint);

    mCellEditUi->setupUi(mCellEditDialog);
    mCellEditDialog->setParent(this);
    mCellEditDialog->setWindowFlags(Qt::Dialog);

    this->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    this->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    mMenuReadOnly = false;

    this->setupConnections();
}

void QEnhancedTableView::setModel(QAbstractItemModel *model)
{
     QTableView::setModel(model);
     TableModel *tbl = (TableModel *)this->model();
     tbl->setView(this);

     QObject::connect(tbl, SIGNAL(cellValueChanged()), this, SLOT(setTabFocus()));
}

void QEnhancedTableView::setMenuReadOnly(bool enabled)
{
    mMenuReadOnly = enabled;
}

void QEnhancedTableView::retranslate()
{
    mHeaderEditUi->retranslateUi(mHeaderEditDialog);
    mCellEditUi->retranslateUi(mCellEditDialog);
}

void QEnhancedTableView::showContextMenu(const QPoint &pos)
{
    QAction* actions[3];
    int increment = 1;
    QMenu myMenu;

    QPoint globalPos = this->viewport()->mapToGlobal(pos);
    TableModel *model = (TableModel*)this->model();

    // Dirty hack
    if (model->getName().contains("AFR"))
    {
        increment = 10;
    }

    if (mMenuReadOnly)
    {
        actions[0] = myMenu.addAction(tr("Clear selection"));
        actions[0]->setIcon(QIcon("://oxygen/32x32/actions/draw-eraser.png"));

        QAction* selectedItem = myMenu.exec(globalPos);
        QModelIndexList selection = this->selectionModel()->selection().indexes();

        // Increase
        if (selectedItem == actions[0])
        {
            for (int i=0; i<selection.size(); i++)
            {
                model->emptyData(selection.at(i));
            }
        }
    }
    else
    {
        actions[0] = myMenu.addAction(tr("Increase by ")+QString::number(increment));
        actions[1] = myMenu.addAction(tr("Decrease by ")+QString::number(increment));
        actions[2] = myMenu.addAction(tr("Change selection..."));

        actions[0]->setIcon(QIcon("://oxygen/32x32/actions/list-add.png"));
        actions[1]->setIcon(QIcon("://oxygen/32x32/actions/list-remove.png"));
        actions[2]->setIcon(QIcon("://oxygen/32x32/actions/quickopen-function.png"));

        QAction* selectedItem = myMenu.exec(globalPos);
        QModelIndexList selection = this->selectionModel()->selection().indexes();

        if (selectedItem == actions[0])
        {
            // Increase
            for (int i=0; i<selection.size(); i++)
            {
                const QModelIndex current = selection.at(i);
                QVariant data(current.data(Qt::EditRole));
                QVariant newdata(data.toInt() + increment);
                model->setData(current, newdata);
            }
        }
        else if (selectedItem == actions[1])
        {
            // Decrease
            for (int i=0; i<selection.size(); i++)
            {
                const QModelIndex current = selection.at(i);
                QVariant data(current.data(Qt::EditRole));
                QVariant newdata(data.toInt() - increment);
                model->setData(current, newdata);
            }
        }
        else if (selectedItem == actions[2])
        {
            // Change using a dialog
            const QPoint globalPos = this->mapToGlobal(pos);
            mCellEditDialog->move(globalPos);
            mCellEditDialog->showNormal();
        }
    }
}

void QEnhancedTableView::clickedVerticalHeader(int section)
{
    if (section < 1 || section >= this->model()->rowCount()-1)
    {
        QMessageBox::information(this, tr("Unable to change"),
                                        tr("This cell cannot be changed"),
                                        QMessageBox::Close);
        return;
    }

    mLastSection = section;
    mLastOrientation = Qt::Vertical;
    const int sectionSize = this->verticalHeader()->sectionSize(0);
    const int verticalOffset = this->verticalHeader()->width();
    const int horizontalOffset = (mHeaderEditDialog->height()/2) - (sectionSize/2);

    const QPoint globalPos = this->verticalHeader()->mapToGlobal(
                QPoint(verticalOffset, (sectionSize*section)-horizontalOffset));

    this->setEditBoundaries(section, Qt::Vertical);
    mHeaderEditDialog->move(globalPos);
    mHeaderEditDialog->showNormal();
}

void QEnhancedTableView::clickedHorizontalHeader(int section)
{
    if (section < 1 || section >= this->model()->columnCount()-1)
    {
        QMessageBox::information(this, tr("Unable to change"),
                                            tr("This cell cannot be changed"),
                                            QMessageBox::Close);
        return;
    }

    mLastSection = section;
    mLastOrientation = Qt::Horizontal;
    const int sectionSize = this->horizontalHeader()->sectionSize(0);
    const int verticalOffset = this->horizontalHeader()->height();
    const int horizontalOffset = (mHeaderEditDialog->width()/2) - (sectionSize/2);

    const QPoint globalPos = this->horizontalHeader()->mapToGlobal(
                QPoint((sectionSize*section)-horizontalOffset, verticalOffset));

    this->setEditBoundaries(section, Qt::Horizontal);
    mHeaderEditDialog->move(globalPos);
    mHeaderEditDialog->showNormal();
}

void QEnhancedTableView::applyChanges()
{
    mHeaderEditDialog->hide();

    QVariant value = mHeaderEditUi->sbValue->value();

    this->model()->setHeaderData(mLastSection, mLastOrientation, value);
}

void QEnhancedTableView::setTabFocus()
{
    emit modelUpdated(this->parentWidget());
}

void QEnhancedTableView::setupConnections()
{
    QObject::connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
    QObject::connect(this->horizontalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(clickedHorizontalHeader(int)));
    QObject::connect(this->verticalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(clickedVerticalHeader(int)));
    QObject::connect(mHeaderEditUi->buttonBox, SIGNAL(accepted()), this, SLOT(applyChanges()));
}

void QEnhancedTableView::setEditBoundaries(int section, Qt::Orientation orientation)
{
    int cur = 0;
    int min = 0;
    int max = 0;
    int margin = 1;
    int minSection = section - 1;
    int maxSection = section + 1;

    if (minSection < 0)
        minSection = 0;
    if (orientation == Qt::Vertical)
    {
        if (maxSection >= this->model()->rowCount())
            maxSection = this->model()->rowCount()-1;
    }
    else
    {
        margin = 100;
        if (maxSection >= this->model()->columnCount())
            maxSection = this->model()->columnCount()-1;
    }

    if (section == 0)
    {
        min = 1;
    }
    else
    {
        min = this->model()->headerData(minSection, orientation).toString().remove("%").toInt()+margin;
    }
    max = this->model()->headerData(maxSection, orientation).toString().remove("%").toInt()-margin;
    cur = this->model()->headerData(section, orientation).toString().remove("%").toInt();

    if (orientation == Qt::Vertical)
    {
        mHeaderEditUi->sbValue->setSingleStep(1);
        mHeaderEditUi->slValue->setSingleStep(1);
    }
    else
    {
        mHeaderEditUi->sbValue->setSingleStep(100);
        mHeaderEditUi->slValue->setSingleStep(100);
    }

    mHeaderEditUi->sbValue->setMinimum(min);
    mHeaderEditUi->sbValue->setMaximum(max);

    mHeaderEditUi->slValue->setMinimum(min);
    mHeaderEditUi->slValue->setMaximum(max);

    mHeaderEditUi->slValue->setValue(cur);
}
