#ifndef COMMANDS_H
#define COMMANDS_H

#include <QUndoCommand>
#include <QStandardItem>
#include <QVariant>

class ModelEditCommand : public QUndoCommand
{
public:
    ModelEditCommand(QStandardItem *item,
                     QVariant value,
                     QStandardItemModel *model,
                     QUndoCommand *parent = 0);

    void undo();
    void redo();

private slots:


private:
    QStandardItem *mItem;
    QVariant mOld, mNew;
    QStandardItemModel *mModel;
};

#endif // COMMANDS_H
