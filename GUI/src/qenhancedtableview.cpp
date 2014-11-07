#include "qenhancedtableview.h"
#include <QDebug>
#include <QMessageBox>
#include <QPainter>
#include <QStaticText>

QEnhancedTableView::QEnhancedTableView(QWidget *parent) :
    QTableView(parent),
    mHeaderEditUi(new Ui::HeaderEdit),
    mHeaderEditDialog(new QDialog)
{
    mHeaderEditUi->setupUi(mHeaderEditDialog);
    mHeaderEditDialog->setParent(this);
    mHeaderEditDialog->setWindowFlags(Qt::Dialog
                                      | Qt::FramelessWindowHint);
    mHeaderEditDialog->setWindowModality(Qt::WindowModal);

    this->setupConnections();
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

void QEnhancedTableView::setupConnections()
{
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

