#include "tablemodel.h"

TableModel::TableModel(QUndoStack *stack, QObject *parent) :
    QStandardItemModel(parent), mStack(stack)
{
    mMin = -30;
    mMax = 30;
    this->fill();
}

bool TableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int val = value.toInt();
    int newvalue = val;
    QStandardItem * item = this->itemFromIndex(index);

    if (val > mMax)
        newvalue = mMax;
    else if (val < mMin)
        newvalue = mMin;

    if (role == Qt::EditRole)
    {
        mStack->push(new ModelEditCommand(item, QVariant(newvalue), this));
    }
    if (role == Qt::UserRole)
        role = Qt::EditRole;
    item->setData(QVariant(QBrush(this->NumberToColor(newvalue+mMax, mMax*2, true))), Qt::BackgroundRole);

    return QStandardItemModel::setData(index, newvalue, role);
}

void TableModel::setMin(int min)
{
    mMin = min;
}

void TableModel::setMax(int max)
{
    mMax = max;
}

QColor TableModel::NumberToColor(float value, float maxValue, bool greenIsNegative)
{
    if (greenIsNegative)
        value = maxValue - value;
    QColor color;
    const float hue = value * (maxValue/20.0) / 360.0;

    color.setHslF(hue, 0.85, 0.25, 0.85);

    return color;
}

void TableModel::fill()
{
    mNumRow = 11;
    mNumCol = 16;
    QColor color(0xFFFFFF);
    QModelIndex * index;

    for (int row = 0; row < mNumRow; ++row)
    {
        for (int column = 0; column < mNumCol; ++column)
        {
            QStandardItem *item = new QStandardItem(0);
            this->setItem(row, column, item);
            this->setHeaderData(column, Qt::Horizontal, (1000*column)+1000);
            this->setHeaderData(row, Qt::Vertical, QString::number(100-(row*10)) + "%");
            //mUi->tableFuel->setColumnWidth(column, 15);

            int rd = qrand() % ((30 + 1) - -30) + -30;
            index = &this->indexFromItem(item);

            this->setData(*index, QVariant(QBrush(Qt::darkGreen)), Qt::BackgroundRole);
            this->setData(*index, QVariant(QBrush(color)), Qt::ForegroundRole );
            this->setData(*index, Qt::AlignCenter, Qt::TextAlignmentRole);
            this->setData(*index, rd, Qt::UserRole);
        }
    }
}
