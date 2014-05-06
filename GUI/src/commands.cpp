#include "commands.h"

ModelEditCommand::ModelEditCommand(QStandardItem *item, QVariant value,
                                 QUndoCommand *parent)
    : QUndoCommand(parent), mItem(item)
{
    mOld = item->index().data();
    mNew = value;
    QString title(QObject::tr("Changed table cell from "));
    title.append(mOld.toString());
    title.append(QObject::tr(" to "));
    title.append(mNew.toString());
    this->setText(title);
}

void ModelEditCommand::undo()
{
    mItem->model()->setData(mItem->index(), mOld);
}

void ModelEditCommand::redo()
{
    mItem->model()->setData(mItem->index(), mNew);
}
