#include "commands.h"

ModelEditCommand::ModelEditCommand(QStandardItem *item, QVariant value,
                                   QStandardItemModel *model,
                                   QUndoCommand *parent)
    : QUndoCommand(parent), mItem(item), mModel(model)
{
    mOld = item->index().data();
    mNew = value;
    QString title(QObject::tr("Changed cell from "));
    title.append(mOld.toString());
    title.append(QObject::tr(" to "));
    title.append(mNew.toString());
    this->setText(title);
}

void ModelEditCommand::undo()
{
    mModel->setData(mItem->index(), mOld, Qt::UserRole);
}

void ModelEditCommand::redo()
{
    mModel->setData(mItem->index(), mNew, Qt::UserRole);
}
