#include "tablemodel.h"
#include <QLineEdit>
#include <QDebug>

TableModel::TableModel(QUndoStack *stack, int min, int max, int def, QObject *parent) :
    QStandardItemModel(parent), mStack(stack)
{
    mMin = min;
    mMax = max;
    mDefault = def;
    this->fill(false);
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
        mStack->push(new ModelEditCommand(item, QVariant(newvalue), mName, this));
    }
    if (role == Qt::UserRole)
        role = Qt::EditRole;
    item->setData(QVariant(QBrush(this->NumberToColor(newvalue, true))), Qt::BackgroundRole);
    item->setData(QVariant(QBrush(Qt::white)), Qt::ForegroundRole );
    item->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

    return QStandardItemModel::setData(index, newvalue, role);
}

bool TableModel::setValue(uint tp, uint rpm, const QVariant &value)
{
    int col, row;
    int maxcol, maxrow;

    maxrow = this->rowCount();
    maxcol = this->columnCount();

    row = -1;
    col = -1;

    for (int i=0; i < maxrow; i++)
    {
        QVariant header = this->headerData(i, Qt::Vertical);
        if (header.toString().remove("%").toUInt() == tp)
        {
            row = i;
            break;
        }
    }

    for (int i=0; i < maxcol; i++)
    {
        QVariant header = this->headerData(i, Qt::Horizontal);
        if (header.toUInt() == rpm)
        {
            col = i;
            break;
        }
    }

    if (row < 0)
        row = 0;
    if (col < 0)
        col = 0;

    if (row > maxrow)
        row = maxrow;
    if (col > maxcol)
        col = maxcol;

    QModelIndex idx = this->index(row, col);

    return this->setData(idx, value, Qt::UserRole);
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
    }
    else {
        value += (range/2.0);
    }
    if (greenIsNegative) /* - Green to + Red */
        value = range - value;

    value /= (range / 280);

    if (value < 0.0)
        value = 0.0;
    const float hue = value * 0.75 / 360.0;
    color.setHslF(hue, 0.70, 0.30, 1.0);

    return color;
}

void TableModel::fill(bool random)
{
    mNumRow = 11;
    mNumCol = 16;
    int value = mDefault;

    for (int row = 0; row < mNumRow; ++row)
    {
        for (int column = 0; column < mNumCol; ++column)
        {
            QStandardItem *item = new QStandardItem(0);
            this->setItem(row, column, item);
            this->setHeaderData(column, Qt::Horizontal, (1000*column)+1000);
            this->setHeaderData(row, Qt::Vertical, QString::number(100-(row*10)) + "%");

            if (random)
                value = mMin + (rand() % (int)(mMax - mMin + 1));

            this->setData(this->indexFromItem(item), value, Qt::UserRole);
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

AfrFormatDelegate::AfrFormatDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

QString AfrFormatDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
    const float display = value.toFloat()/10;

    return locale.toString(display, 'f', 1);
}

QWidget * AfrFormatDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QWidget* editor = QStyledItemDelegate::createEditor(parent, option, index);
    QLineEdit* lineEditEditor = qobject_cast<QLineEdit*>(editor);
    if( lineEditEditor )
        lineEditEditor->setValidator(&mValidator);

    return editor;
}
