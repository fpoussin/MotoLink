#ifndef SPINBOX_H
#define SPINBOX_H

#include <QSpinBox>
#include <QUndoStack>
#include "commands.h"

class SpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit SpinBox(QWidget *parent = 0);
    void setUndoStack(QUndoStack *stack);
    void setName(QString name);

signals:

public slots:
    void pushUndo(void);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    int mOld;
    QString mName;
    QUndoStack *mUndoStack;

};

#endif // SPINBOX_H
