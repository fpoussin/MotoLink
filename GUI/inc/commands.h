#ifndef COMMANDS_H
#define COMMANDS_H

#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QStandardItem>
#include <QUndoCommand>
#include <QVariant>

namespace Commands {

const QString msgChanged(QObject::tr("Changed"));
const QString msgFrom(QObject::tr("from"));
const QString msgTo(QObject::tr("to"));
}

class ModelEditCommand : public QUndoCommand {
public:
  ModelEditCommand(QStandardItem *item, QVariant value, QString &name,
                   QStandardItemModel *model, QUndoCommand *parent = 0);

public slots:
  void undo();
  void redo();

private:
  QStandardItem *mItem;
  QVariant mOld, mNew;
  QStandardItemModel *mModel;
};

class SpinBoxEditCommand : public QUndoCommand {
public:
  SpinBoxEditCommand(QSpinBox *spinbox, int value, QString &name,
                     QUndoCommand *parent = 0);

public slots:
  void undo();
  void redo();

private:
  QSpinBox *mSpinBox;
  int mOld, mNew;
};

class DoubleSpinBoxEditCommand : public QUndoCommand {
public:
  DoubleSpinBoxEditCommand(QDoubleSpinBox *spinbox, double value, QString &name,
                           QUndoCommand *parent = 0);

public slots:
  void undo();
  void redo();

private:
  QDoubleSpinBox *mSpinBox;
  double mOld, mNew;
};

#endif // COMMANDS_H
