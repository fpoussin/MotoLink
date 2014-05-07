#include "inc/spinbox.h"

SpinBox::SpinBox(QWidget *parent) :
    QSpinBox(parent)
{
    mUndoStack = NULL;
    mName = "SpinBox";
    mOld = this->value();

    QObject::connect(this, SIGNAL(editingFinished()), this, SLOT(pushUndo()));
}

void SpinBox::setUndoStack(QUndoStack *stack)
{
    mOld = this->value();
    mUndoStack = stack;
}

void SpinBox::setName(QString name)
{
    mName = name;
}

void SpinBox::pushUndo()
{
    if (this->value() == mOld)
        return;
    if (mUndoStack != NULL)
    {
        /* Pushes when undoing as well... */
        mUndoStack->push(new SpinBoxEditCommand(this, mOld, mName));
    }
    mOld = this->value();
}
