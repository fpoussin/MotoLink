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
    item->setData(QVariant(QBrush(this->NumberToColor(newvalue, true))), Qt::BackgroundRole);
    item->setData(QVariant(QBrush(Qt::white)), Qt::ForegroundRole );
    item->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

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

QColor TableModel::NumberToColor(float value, bool greenIsNegative)
{
    value += mMax;
    if (greenIsNegative) /* Green to Red */
        value = (mMax*2) - value;
    QColor color;
    const float hue = value * (mMax/10.0) / 360.0;

    color.setHslF(hue, 0.85, 0.40, 0.80);

    return color;
}

void TableModel::fill()
{
    mNumRow = 11;
    mNumCol = 16;

    for (int row = 0; row < mNumRow; ++row)
    {
        for (int column = 0; column < mNumCol; ++column)
        {
            QStandardItem *item = new QStandardItem(0);
            this->setItem(row, column, item);
            this->setHeaderData(column, Qt::Horizontal, (1000*column)+1000);
            this->setHeaderData(row, Qt::Vertical, QString::number(100-(row*10)) + "%");

            int rd = qrand() % ((mMax + 1) - -mMax) + -mMax;
            this->setData(this->indexFromItem(item), rd, Qt::UserRole);
        }
    }
}
