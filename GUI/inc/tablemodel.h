#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QStandardItemModel>
#include <QUndoStack>
#include <QString>
#include <QStyledItemDelegate>
#include <QValidator>
#include "commands.h"

class TableModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit TableModel(QUndoStack *stack,QObject *parent = 0);
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QString getName(void) { return mName; }
    int getMin(void) { return mMin; }
    int getMax(void) { return mMax; }

signals:

public slots:
    void setName(const QString name);
    void setMin(int min);
    void setMax(int max);

private:
    QColor NumberToColor(float value, bool greenIsNegative);
    void fill(bool random = false);

    QUndoStack* mStack;
    QString mName;
    QString mSuffix;
    int mMin;
    int mMax;
    quint8 mNumCol;
    quint8 mNumRow;

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

#endif // TABLEMODEL_H
