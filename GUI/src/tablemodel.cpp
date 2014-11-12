#include "tablemodel.h"
#include <QLineEdit>
#include <QDebug>

TableModel::TableModel(QUndoStack *stack, int min, int max, int def, QObject *parent) :
    QStandardItemModel(parent),
    mStack(stack)
{
    mMin = min;
    mMax = max;
    mDefaultValue = def;
    mLastItem = NULL;
    mView = NULL;
    this->fill(false);
}

TableModel::~TableModel()
{
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
        emit cellValueChanged();
    }
    if (role == Qt::UserRole)
    {
        role = Qt::EditRole;
        emit cellValueChanged();
    }
    item->setData(this->NumberToColor(newvalue, true), Qt::BackgroundRole);
    item->setData(QColor(Qt::white), Qt::ForegroundRole);
    item->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

    return QStandardItemModel::setData(index, newvalue, role);
}

void TableModel::emptyData(const QModelIndex &index)
{
    QStandardItem * item = this->itemFromIndex(index);
    item->setData(QColor(Qt::white), Qt::BackgroundRole);
    QStandardItemModel::setData(index, QVariant(""), Qt::EditRole);
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant data(QStandardItemModel::headerData(section, orientation, role));
    if (role == Qt::DisplayRole && orientation == Qt::Vertical)
    {
        QString str(data.toString());
        str.append("%");
        data = str;
    }

    return data;
}

bool TableModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    QVariant tmp(value);
    if (orientation == Qt::Horizontal)
    {
        int tmpval = tmp.toInt();
        tmpval /= 100;
        tmpval *= 100;
        tmp.setValue(tmpval);
    }

    if (role != Qt::UserRole) {
        emit headerDataNeedSync(section, orientation, value);
    }
    else {
        role = Qt::EditRole;
    }
    return QStandardItemModel::setHeaderData(section, orientation, tmp, role);
}

bool TableModel::setValue(uint row, uint col, const QVariant &value)
{
    QModelIndex idx = this->index(row, col);

    return this->setData(idx, value, Qt::UserRole);
}

void TableModel::highlightCell(int row, int col)
{
    QFont font;
    QStandardItem* item = this->item(row, col);
    float color = item->data(Qt::EditRole).toFloat();
    if (item == NULL)
        return;

    if (mLastItem != NULL && mLastItem != item)
    {
        mLastItem->setData(QVariant(this->NumberToColor(color)), Qt::BackgroundRole);
        mLastItem->setData(QVariant(QColor(Qt::white)), Qt::ForegroundRole);
        mLastItem->setData(QVariant(font), Qt::FontRole);
    }

    font.setBold(true);
    item->setData(QVariant(this->NumberToColor(color, true, false)), Qt::BackgroundRole);
    item->setData(QVariant(QColor(Qt::black)), Qt::ForegroundRole);
    item->setData(QVariant(font), Qt::FontRole);
    mLastItem = item;
}

bool TableModel::getCell(uint tp, uint rpm, int *row, int *col)
{
    int maxcol, maxrow;

    *row = -1;
    *col = -1;

    maxrow = this->rowCount()-1;
    maxcol = this->columnCount()-1;

    for (int i=0; i < maxrow; i++)
    {
        uint h = this->headerData(i, Qt::Vertical, Qt::EditRole).toUInt();
        uint h2 = this->headerData(i+1, Qt::Vertical, Qt::EditRole).toUInt();

        if (tp >= this->headerData(maxrow, Qt::Vertical, Qt::EditRole).toUInt())
        {
            *row = maxrow;
            break;
        }
        else if (tp >= h && tp <= h2)
        {
            *row = i;
            break;
        }
    }

    for (int i=0; i < maxcol; i++)
    {
        uint h = this->headerData(i, Qt::Horizontal, Qt::EditRole).toUInt();
        uint h2 = this->headerData(i+1, Qt::Horizontal, Qt::EditRole).toUInt();

        if (rpm >= this->headerData(maxcol, Qt::Horizontal, Qt::EditRole).toUInt())
        {
            *col = maxcol;
            break;
        }
        if (rpm >= h && rpm <= h2)
        {
            *col = i;
            break;
        }
    }

    return (*row >= 0 && *col >= 0);
}

void TableModel::setView(QEnhancedTableView *view)
{
    mView = view;
}

QEnhancedTableView *TableModel::view()
{
    return mView;
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

QColor TableModel::NumberToColor(float value, bool greenIsNegative, bool darkColor)
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
    if (darkColor)
        color.setHslF(hue, 0.70, 0.30, 1.0);
    else
        color.setHslF(hue, 1.0, 0.8, 1.0);

    return color;
}

void TableModel::fill(bool random)
{
    mNumRow = 11;
    mNumCol = 16;
    int value = mDefaultValue;

    for (int row = 0; row < mNumRow; ++row)
    {
        for (int column = 0; column < mNumCol; ++column)
        {
            QStandardItem *item = new QStandardItem(0);
            this->setItem(row, column, item);
            this->setHeaderData(column, Qt::Horizontal, mHrc.getDefaultRpmAt(column));
            this->setHeaderData(row, Qt::Vertical, mHrc.getDefaultTpsAt(row));

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
    bool ok;
    const float display = value.toFloat(&ok)/10;

    if (ok)
        return locale.toString(display, 'f', 1);
    return QString();
}

QWidget * AfrFormatDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QWidget* editor = QStyledItemDelegate::createEditor(parent, option, index);
    QLineEdit* lineEditEditor = qobject_cast<QLineEdit*>(editor);
    if( lineEditEditor )
        lineEditEditor->setValidator(&mValidator);

    return editor;
}
