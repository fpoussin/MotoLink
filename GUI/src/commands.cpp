#include "commands.h"

ModelEditCommand::ModelEditCommand(QStandardItemModel *model,
                                 QUndoCommand *parent)
    : QUndoCommand(parent)
{
    mModel = model;

    setText(QObject::tr("Change table"));
}

void ModelEditCommand::undo()
{

}

void ModelEditCommand::redo()
{

}
