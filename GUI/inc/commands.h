#ifndef COMMANDS_H
#define COMMANDS_H

#include <QUndoCommand>
#include <QStandardItem>
#include <QPoint>
#include <QVariant>

class ModelEditCommand : public QUndoCommand
{
public:
    ModelEditCommand(QStandardItemModel *model,
                    QUndoCommand *parent = 0);

    void undo();
    void redo();

private slots:


private:
    QStandardItemModel *mModel;
    QStandardItemModel *mOldModel;
};

#endif // COMMANDS_H
