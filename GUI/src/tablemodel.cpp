#include "tablemodel.h"
#include <QLineEdit>

TableModel::TableModel(QUndoStack *stack, int min, int max, QObject *parent) :
    QStandardItemModel(parent), mStack(stack)
{
    mMin = min;
    mMax = max;
    this->fill(true);
}

bool TableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    int val = value.toInt();
    int newvalue = val;
    QStandardItem * item = this->itemFromIndex(index);
    QString valueStr;

    if (val > mMax)
        newvalue = mMax;
    else if (val < mMin)
        newvalue = mMin;

    if (role == Qt::EditRole)
    {
        mStack->push(new ModelEditCommand(item, QVariant(newvalue), mName, this));
    }
    if (role == Qt::UserRole)
        role = Qt::EditRole;
    item->setData(QVariant(QBrush(this->NumberToColor(newvalue, true))), Qt::BackgroundRole);
    item->setData(QVariant(QBrush(Qt::white)), Qt::ForegroundRole );
    item->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

    return QStandardItemModel::setData(index, newvalue, role);
}

void TableModel::setName(const QString name)
{
    mName = name;
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
    QColor color;
    const float range = mMax - mMin;

    if (mMin > 0) {
        value -= mMin;
        //value -= (range*3.0);
    }
    else {
        value += (range/2.0);
    }
    if (greenIsNegative) /* - Green to + Red */
        value = range - value;

    value /= (range / 250.0);

    if (value < 0)
        value = 0;
    const float hue = value * 0.75 / 360.0;
    color.setHslF(hue, 0.70, 0.30, 1.0);

    return color;
}

void TableModel::fill(bool random)
{
    mNumRow = 11;
    mNumCol = 16;
    int rd = 0;

    for (int row = 0; row < mNumRow; ++row)
    {
        for (int column = 0; column < mNumCol; ++column)
        {
            QStandardItem *item = new QStandardItem(0);
            this->setItem(row, column, item);
            this->setHeaderData(column, Qt::Horizontal, (1000*column)+1000);
            this->setHeaderData(row, Qt::Vertical, QString::number(100-(row*10)) + "%");

            if (random)
                rd = qrand() % ((mMax + 1) - -mMax) + -mMax;

            this->setData(this->indexFromItem(item), rd, Qt::UserRole);
        }
    }
}

NumberFormatDelegate::NumberFormatDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

QString NumberFormatDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    if (value.toInt() > 0)
        return QString("+")+locale.toString(value.toInt())+mSuffix;

    return locale.toString(value.toInt())+mSuffix;
}

QWidget * NumberFormatDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QWidget* editor = QStyledItemDelegate::createEditor(parent, option, index);
    QLineEdit* lineEditEditor = qobject_cast<QLineEdit*>(editor);
    if( lineEditEditor )
        lineEditEditor->setValidator(&mValidator);

    return editor;
}
