#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QStandardItemModel>
#include <QUndoStack>
#include <QString>
#include <QStyledItemDelegate>
#include <QValidator>
#include "qenhancedtableview.h"
#include "commands.h"


class TableModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit TableModel(QUndoStack *stack, int min = -30, int max = 30, int def = 0, bool singlerow = false, bool permanent = true, QObject *parent = 0);
    ~TableModel(void);
    bool isPermanent(void);
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    void setDataFromArray(const quint8 * array);
    void emptyData(const QModelIndex &index);
    QEnhancedTableView * view();
    void setSingleRow(bool val);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);
    QString getName(void) { return mName; }
    int getMin(void) { return mMin; }
    int getMax(void) { return mMax; }
    bool setValue(uint row, uint col, const QVariant &value);
    bool writeCellPeak(uint tp, uint rpm, QVariant &value);
    bool writeCellAverage(uint tp, uint rpm, QVariant &value);
    void highlightCell(int row, int col);
    bool getCell(uint tp, uint rpm, int *row, int *col);
    void setView(QEnhancedTableView *view);
    void setId(uint id);
    uint id(void) { return mId; }
    void rowsToArray(quint8 * data, int maxLen);
    void arrayToRows(const quint8 * data, int maxLen);
    void columnsToArray(quint8 * data, int maxLen);
    void arrayToColumns(const quint8 * data, int maxLen);
    int getDefaultRpmAt(int index);
    int getDefaultTpsAt(int index);

signals:
    void headerDataNeedSync(int, Qt::Orientation, const QVariant);
    void cellValueChanged(int row, int col);
    void cellCleared(uint id, int row, int col);

public slots:
    void setName(const QString name);
    void setMin(int min);
    void setMax(int max);

private slots:

private:
    QColor NumberToColor(float value, bool greenIsNegative = true, bool darkColor = true);
    void fill(bool random = false);

    QEnhancedTableView *mView;
    QUndoStack* mStack;
    QString mName;
    QString mSuffix;
    int mMin;
    int mMax;
    int mDefaultValue;
    quint8 mNumCol;
    quint8 mNumRow;
    QStandardItem* mLastItem;
    bool mPermanent;
    bool mSinglerow;
    uint mId;
    QVector<int> mDefaultRpm;
    QVector<int> mDefaultTps;
};

class NumberFormatDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit NumberFormatDelegate(QObject *parent = 0);
    virtual QString displayText(const QVariant &value, const QLocale &locale) const;
    QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
    void setSuffix(QString suffix) { mSuffix = suffix;}

signals:

public slots:

private:
    QIntValidator mValidator;
    QString mSuffix;

};

class AfrFormatDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit AfrFormatDelegate(QObject *parent = 0);
    virtual QString displayText(const QVariant &value, const QLocale &locale) const;
    QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;

signals:

public slots:

private:
    QDoubleValidator mValidator;

};

#endif // TABLEMODEL_H
