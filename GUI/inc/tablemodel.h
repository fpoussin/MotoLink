#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QStandardItemModel>
#include <QUndoStack>
#include <QString>
#include <QStyledItemDelegate>
#include <QValidator>
#include "qenhancedtableview.h"
#include "commands.h"
#include "hrc.h"


class TableModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit TableModel(QUndoStack *stack, int min = -30, int max = 30, int def = 0, QObject *parent = 0);
    ~TableModel(void);
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    void emptyData(const QModelIndex &index);
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);
    QString getName(void) { return mName; }
    int getMin(void) { return mMin; }
    int getMax(void) { return mMax; }
    bool setValue(uint row, uint col, const QVariant &value);
    void highlightCell(int row, int col);
    bool getCell(uint tp, uint rpm, int *row, int *col);
    void setView(QEnhancedTableView *view);
    QEnhancedTableView * view();

signals:
    void headerDataNeedSync(int, Qt::Orientation, const QVariant);
    void cellValueChanged(void);

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
    Hrc mHrc;
    QStandardItem* mLastItem;

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
