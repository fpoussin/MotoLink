#include "spinbox.h"

#include <QCloseEvent>
#include <qcoreevent.h>
#include <QDebug>

SpinBox::SpinBox(QWidget *parent)
    : QSpinBox(parent)
{
    mUndoStack = NULL;
    mName = "SpinBox";
    mOld = this->value();
    this->installEventFilter(this);

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
    if (mUndoStack != NULL && !this->hasFocus()) {
        mUndoStack->push(new SpinBoxEditCommand(this, mOld, mName));
    }
    mOld = this->value();
}

bool SpinBox::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvt = (QKeyEvent *)event;
        const int numKey = keyEvt->key();
        const bool isCtrl = keyEvt->modifiers().testFlag(Qt::ControlModifier);

        if (numKey == Qt::Key_Z && isCtrl) {
            this->setFocus();
            this->mUndoStack->undo();
            event->accept();
            return true;
        }

        else if (numKey == Qt::Key_Y && isCtrl) {
            this->setFocus();
            this->mUndoStack->redo();
            event->accept();
            return true;
        }

        else if (numKey == Qt::Key_Escape
                 || numKey == Qt::Key_Enter
                 || numKey == Qt::Key_Return) {
            this->clearFocus();
            event->accept();
            return true;
        }
    }

    return QSpinBox::eventFilter(obj, event);
}
