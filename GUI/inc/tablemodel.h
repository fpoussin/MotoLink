#ifndef TABLEMODEL_H
#define TABLEMODEL_H

#include <QStandardItemModel>
#include <QUndoStack>
#include "commands.h"

class TableModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit TableModel(QUndoStack *stack, QObject *parent = 0);
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    void setMin(int min);
    void setMax(int max);

signals:

public slots:


private:
    QColor NumberToColor(float value, float maxValue, bool greenIsNegative);
    void fill(void);

    QUndoStack* mStack;
    int mMin;
    int mMax;
    quint8 mNumCol;
    quint8 mNumRow;

};

#endif // TABLEMODEL_H
