#include "spinbox.h"

#include <QCloseEvent>
#include <qcoreevent.h>
#include <QDebug>

SpinBox::SpinBox(QWidget *parent) :
    QSpinBox(parent)
{
    mUndoStack = NULL;
    mName = "SpinBox";
    mOld = this->value();
    this->installEventFilter(this);
    this->setFocusPolicy( Qt::StrongFocus );

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
        /* Pushes when undoing with focus as well... */
        mUndoStack->push(new SpinBoxEditCommand(this, mOld, mName));
    }
    mOld = this->value();
}

bool SpinBox::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvt= (QKeyEvent *)event;
        if(keyEvt->key() == Qt::Key_Z
                && keyEvt->modifiers().testFlag(Qt::ControlModifier))
        {
            this->clearFocus();
            this->mUndoStack->undo();
            this->setFocus();
            event->accept();
            return true;
        }

        else if(keyEvt->key() == Qt::Key_Y
                && keyEvt->modifiers().testFlag(Qt::ControlModifier))
        {
            this->clearFocus();
            this->mUndoStack->redo();
            this->setFocus();
            event->accept();
            return true;
        }

        else if(keyEvt->key() == Qt::Key_Escape)
        {
            this->clearFocus();
            event->accept();
            return true;
        }
    }

    return QSpinBox::eventFilter(obj, event);
}
