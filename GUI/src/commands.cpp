#include "commands.h"
#include <QToolTip>

ModelEditCommand::ModelEditCommand(QStandardItem *item, QVariant value,
                                   QString &name, QStandardItemModel *model,
                                   QUndoCommand *parent)
    : QUndoCommand(parent), mItem(item), mModel(model) {
  mOld = item->index().data();
  mNew = value;

  const QString colname =
      model->headerData(item->index().column(), Qt::Horizontal).toString();

  const QString rowname =
      model->headerData(item->index().row(), Qt::Vertical).toString();

  using namespace Commands;
  QString title = QString("%1 %2 %3:%4 %5 %6 %7 %8")
                      .arg(msgChanged, name, rowname, colname, msgFrom,
                           mOld.toString(), msgTo, mNew.toString());

  this->setText(title);
}

void ModelEditCommand::undo() {
  mModel->setData(mItem->index(), mOld, Qt::UserRole);
}

void ModelEditCommand::redo() {
  mModel->setData(mItem->index(), mNew, Qt::UserRole);
}

SpinBoxEditCommand::SpinBoxEditCommand(QSpinBox *spinbox, int value,
                                       QString &name, QUndoCommand *parent)
    : QUndoCommand(parent) {
  mSpinBox = spinbox;
  mNew = spinbox->value();
  mOld = value;

  using namespace Commands;
  QString title = QString("%1 %2 %3 %4 %5 %6")
                      .arg(msgChanged, name, msgFrom, QString::number(mOld),
                           msgTo, QString::number(mNew));

  this->setText(title);
}

void SpinBoxEditCommand::undo() {
  QToolTip::showText(mSpinBox->mapToGlobal(QPoint(0, 0)), "Undo");
  mSpinBox->setValue(mOld);
}

void SpinBoxEditCommand::redo() {
  QToolTip::showText(mSpinBox->mapToGlobal(QPoint(0, 0)), "Redo");
  mSpinBox->setValue(mNew);
}

DoubleSpinBoxEditCommand::DoubleSpinBoxEditCommand(QDoubleSpinBox *spinbox,
                                                   double value, QString &name,
                                                   QUndoCommand *parent)
    : QUndoCommand(parent) {
  mSpinBox = spinbox;
  mNew = spinbox->value();
  mOld = value;

  using namespace Commands;
  QString title = QString("%1 %2 %3 %4 %5 %6")
                      .arg(msgChanged, name, msgFrom, QString::number(mOld),
                           msgTo, QString::number(mNew));

  this->setText(title);
}

void DoubleSpinBoxEditCommand::undo() {
  QToolTip::showText(mSpinBox->mapToGlobal(QPoint(0, 0)), "Undo");
  mSpinBox->setValue(mOld);
}

void DoubleSpinBoxEditCommand::redo() {
  QToolTip::showText(mSpinBox->mapToGlobal(QPoint(0, 0)), "Redo");
  mSpinBox->setValue(mNew);
}
