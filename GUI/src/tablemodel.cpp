#include "tablemodel.h"
#include <QLineEdit>
#include <QDebug>

TableModel::TableModel(QUndoStack *stack, float min, float max, float def, bool singlerow, bool permanent, QObject *parent) :
    QStandardItemModel(parent),
    mStack(stack)
{
    mMin = min;
    mMax = max;
    mDefaultValue = def;
    mLastItem = NULL;
    mView = NULL;
    mPermanent = permanent;
    mSinglerow = singlerow;
    this->mId = 0;

    const int defaultRpm[] = {0, 2000, 4000, 5500, 7000, 8000, 9000,
                  10000, 11000, 12000, 13000, 14000, 15000, 16000,
                  16500, 18000};

    const int defaultTps[] = {0, 4, 8, 15, 20, 27, 35, 50, 70, 85, 100};

    for (uint i=0; i< sizeof(defaultRpm)/sizeof(int); i++)
    {
        mDefaultRpm.append(defaultRpm[i]);
    }

    for (uint i=0; i< sizeof(defaultTps)/sizeof(int); i++)
    {
        mDefaultTps.append(defaultTps[i]);
    }

    this->fill(false);
}

TableModel::~TableModel()
{
}

bool TableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    float val = value.toFloat();
    float newvalue = val;
    QStandardItem * item = this->itemFromIndex(index);
    bool notify = false;
    bool result;

    if (item == NULL)
        return false;

    if (val == item->data(Qt::EditRole))
      return false;

    if (val > mMax)
        newvalue = mMax;
    else if (val < mMin)
        newvalue = mMin;

    /* Manual cell edit, add action to undo/redo log */
    if (role == Qt::EditRole)
    {
        mStack->push(new ModelEditCommand(item, QVariant(newvalue), mName, this));
        notify = true;
    }
    /* Read only cell edit - No extra action */
    else if (role == Qt::UserRole)
    {
        role = Qt::EditRole;
        notify = true;
    }
    /* Silent edit, for full tables update */
    else
    {
        qWarning("Unknown cell value edit role: %d", role);
    }
    item->setData(this->NumberToColor(newvalue, true), Qt::BackgroundRole);
    item->setData(QColor(Qt::white), Qt::ForegroundRole);
    item->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

    result = QStandardItemModel::setData(index, newvalue, role);

    if (notify)
      emit cellValueChanged(index.row(), index.column());

    return result;
}

void TableModel::setDataFromArray(const quint8 *array, float multiplier)
{
    QModelIndex idx;
    float data;

    for (int i=0; i<this->rowCount(); i++)
    {
        for (int j=0; j<this->columnCount(); j++)
        {
            idx = this->index(i, j);
            data = (float)array[(i*this->columnCount())+j] * multiplier;

            /* Skip if data unchanged */
            if (data == this->data(idx, Qt::EditRole)) continue;

            if (idx.isValid() && data > 0)
            {
                this->setData(idx, QVariant(data), Qt::UserRole);
            }
            else
            {
                QStandardItem * item = this->itemFromIndex(idx);
                if (item == NULL)
                    continue;
                item->setData(QColor(Qt::gray), Qt::BackgroundRole);
                QStandardItemModel::setData(idx, QVariant(), Qt::EditRole);
            }
        }
    }
}

void TableModel::emptyData(const QModelIndex &index)
{
    QStandardItem * item = this->itemFromIndex(index);
    if (item == NULL)
        return;
    item->setData(QColor(Qt::white), Qt::BackgroundRole);
    QStandardItemModel::setData(index, QVariant(), Qt::EditRole);
    emit cellCleared(mId, index.row(), index.column());
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant data(QStandardItemModel::headerData(section, orientation, role));
    if (role == Qt::DisplayRole && orientation == Qt::Vertical)
    {
        if (mSinglerow)
            data = "Value";
        else
        {
            QString str(data.toString());
            str.append("%");
            data = str;
        }
    }

    return data;
}

bool TableModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    QVariant tmp(value);
    if (orientation == Qt::Horizontal)
    {
        /* Average to +- 100 */
        int tmpval = tmp.toInt();
        tmpval /= 100;
        tmpval *= 100;
        tmp.setValue(tmpval);
    }

    if (mSinglerow && orientation == Qt::Vertical)
        return true;

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
    if (!idx.isValid())
        return false;

    return this->setData(idx, value, Qt::UserRole);
}

bool TableModel::writeCellPeak(uint tp, uint rpm, QVariant &value)
{
    int row = -1;
    int col = -1;
    if (!this->getCell(tp, rpm, &row, &col))
        return false;

    QStandardItem* item = this->item(row, col);
    QVariant old(item->data(Qt::EditRole));

    if (old > value)
        item->setData(old, Qt::EditRole);
    else
        item->setData(value, Qt::EditRole);
    item->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

    return true;
}

bool TableModel::writeCellAverage(uint tp, uint rpm, QVariant &value)
{
    int row = -1;
    int col = -1;

    if (!this->getCell(tp, rpm, &row, &col))
        return false;

    QStandardItem* item = this->item(row, col);
    QVariant average(item->data(Qt::EditRole));

    if (!average.isNull())
    {
        float flAvg = average.value<float>();

        flAvg += value.toFloat();
        flAvg /= 2;
        item->setData(QVariant(flAvg), Qt::EditRole);
    }
    else
    {
        item->setData(value, Qt::EditRole);
    }
    item->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

    return true;
}

void TableModel::highlightCell(int row, int col)
{
    QFont font;
    if (mSinglerow)
        row = 0;
    QStandardItem* item = this->item(row, col);
    if (item == NULL)
        return;

    QString valueStr = item->data(Qt::EditRole).value<QString>();
    float value = item->data(Qt::EditRole).toFloat();

    if (mLastItem != NULL && mLastItem != item)
    {
        if (valueStr.isEmpty())
            mLastItem->setData(QColor(Qt::white), Qt::BackgroundRole);
        else
            mLastItem->setData(this->NumberToColor(value), Qt::BackgroundRole);

        mLastItem->setData(QColor(Qt::white), Qt::ForegroundRole);
        mLastItem->setData(QVariant(font), Qt::FontRole);
    }

    font.setBold(true);

    if (valueStr.isEmpty())
        item->setData(QColor(Qt::gray), Qt::BackgroundRole);
    else
        item->setData(this->NumberToColor(value, true, false), Qt::BackgroundRole);
    item->setData(QColor(Qt::black), Qt::ForegroundRole);
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

    if (mSinglerow)
    {
        *row = 0;
    }
    else
    {
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
        else if (rpm >= h && rpm <= h2)
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

void TableModel::setId(uint id)
{
    mId = id;
}

void TableModel::rowsToArray(quint8 *data, int maxLen)
{
    bool ok;
    uint value;
    for (int i=0; i < this->rowCount() && i < maxLen; i++)
    {
        ok = false;
        value = this->headerData(i, Qt::Vertical, Qt::EditRole).toUInt(&ok);
        if (ok)
            data[i] = value & 0xFF;
    }
}

void TableModel::arrayToRows(const quint8 *data, int maxLen)
{
    Q_CHECK_PTR(data);
    for (int i=0; i < this->rowCount() && i < maxLen; i++)
    {
        this->setHeaderData(i, Qt::Vertical, data[i], Qt::EditRole);
    }
}

void TableModel::columnsToArray(quint8 *data, int maxLen)
{
    bool ok;
    uint value;
    for (int i=0; i < this->columnCount() && i < maxLen; i++)
    {
        ok = false;
        value = this->headerData(i, Qt::Horizontal, Qt::EditRole).toUInt(&ok)/100;
        if (ok)
            data[i] = value & 0xFF;
    }
}

void TableModel::arrayToColumns(const quint8 *data, int maxLen)
{
    Q_CHECK_PTR(data);
    for (int i=0; i < this->columnCount() && i < maxLen; i++)
    {
        this->setHeaderData(i, Qt::Horizontal, data[i]*100, Qt::EditRole);
    }
}

void TableModel::setSingleRow(bool val)
{
    mSinglerow = val;
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

int TableModel::getDefaultRpmAt(int index)
{
    return mDefaultRpm.at(index);
}

int TableModel::getDefaultTpsAt(int index)
{
    return mDefaultTps.at(index);
}

void TableModel::fill(bool random)
{
    mNumRow = 11;
    mNumCol = 16;
    float value = mDefaultValue;

    if (mSinglerow)
        mNumRow = 1;

    for (int row = 0; row < mNumRow; row++)
    {
        for (int column = 0; column < mNumCol; column++)
        {
            QStandardItem *item = new QStandardItem(0);
            this->setItem(row, column, item);
            this->setHeaderData(column, Qt::Horizontal, getDefaultRpmAt(column));
            this->setHeaderData(row, Qt::Vertical, getDefaultTpsAt(row));

            if (random)
                value = mMin + (rand() % (int)(mMax - mMin + 1));
            if (mPermanent) /* Only fill cells if data is not volatile */
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
    const float display = value.toFloat(&ok);

    if (ok)
        return locale.toString(display, 'f', 2);

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
