#include "commands.h"

ModelEditCommand::ModelEditCommand(QStandardItem *item, QVariant value,
                                   QStandardItemModel *model,
                                   QUndoCommand *parent)
    : QUndoCommand(parent), mItem(item), mModel(model)
{
    mOld = item->index().data();
    mNew = value;

    /* For easy translation */
    const QString cellMsg(QObject::tr("Changed cell"));
    const QString cellFrom(QObject::tr("from"));
    const QString cellTo(QObject::tr("to"));

    const QString colname = model->headerData(
                item->index().column(), Qt::Horizontal).toString();

    const QString rowname = model->headerData(
                item->index().row(), Qt::Vertical).toString();

    QString title = QString(cellMsg+" %1:%2 "+cellFrom+" %3 "+cellTo+" %4")
            .arg(rowname, colname, mOld.toString(), mNew.toString());

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
