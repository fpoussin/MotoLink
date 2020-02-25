// Qt includes
#include <QApplication>
#include <QEvent>
#include <QLabel>
#include <QLayout>
#include <QPainter>
#include <QTabBar>
#include <QTabWidget>
#include <QBoxLayout>
#include <QMessageBox>
#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QScreen>
#include <QAction>
#include <QMenu>

#include "mhtabbar.h"
#include "mhtabwidget.h"

//////////////////////////////////////////////////////////////
void MHTabBar::Initialize(QMainWindow * /*mainWindow*/)
{
}

//////////////////////////////////////////////////////////////
void MHTabBar::ShutDown(void)
{
}

//////////////////////////////////////////////////////////////
// Default Constructor
//////////////////////////////////////////////////////////////
MHTabBar::MHTabBar(QWidget *parent)
    : QTabBar(parent)
{
    this->setAcceptDrops(true);

    this->setElideMode(Qt::ElideRight);
    this->setSelectionBehaviorOnRemove(QTabBar::SelectLeftTab);

    this->setMovable(true);

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(onRightClick(QPoint)));
}

//////////////////////////////////////////////////////////////
// Default Destructor
//////////////////////////////////////////////////////////////
MHTabBar::~MHTabBar(void)
{
}

//////////////////////////////////////////////////////////////////////////////
void MHTabBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragStartPos = event->pos();
    }
    m_dragDropedPos.setX(0);
    m_dragDropedPos.setY(0);
    m_dragMovedPos.setX(0);
    m_dragMovedPos.setY(0);

    m_dragInitiated = false;

    QTabBar::mousePressEvent(event);
}

//////////////////////////////////////////////////////////////////////////////
void MHTabBar::mouseMoveEvent(QMouseEvent *event)
{
    // Distinguish a drag
    if (!m_dragStartPos.isNull() && ((event->pos() - m_dragStartPos).manhattanLength() < QApplication::startDragDistance())) {
        m_dragInitiated = true;
    }

    // The left button is pressed
    // And the move could also be a drag
    // And the mouse moved outside the tab bar
    if (((event->buttons() & Qt::LeftButton)) && m_dragInitiated && (!geometry().contains(event->pos()))) {
        // Stop the move to be able to convert to a drag
        {
            QMouseEvent *finishMoveEvent = new QMouseEvent(QEvent::MouseMove, event->pos(), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
            QTabBar::mouseMoveEvent(finishMoveEvent);
            delete finishMoveEvent;
            finishMoveEvent = NULL;
        }

        // Initiate Drag
        QDrag *drag = new QDrag(this);
        QMimeData *mimeData = new QMimeData;
        // a crude way to distinguish tab-reordering drops from other ones
        mimeData->setData("action", "application/tab-detach");
        drag->setMimeData(mimeData);

        // Create transparent screen dump
        QScreen *screen = QApplication::screens().at(0);
        QPixmap pixmap = screen->grabWindow(dynamic_cast<MHTabWidget *>(parentWidget())->currentWidget()->winId()).scaled(400, 200, Qt::KeepAspectRatio);
        QPixmap targetPixmap(pixmap.size());
        QPainter painter(&targetPixmap);
        painter.setOpacity(0.5);
        painter.drawPixmap(0, 0, pixmap);
        painter.end();
        drag->setPixmap(targetPixmap);
        //    drag->setHotSpot(QPoint (20, 10));

        // Handle Detach and Move
        Qt::DropAction dragged = drag->exec(Qt::MoveAction | Qt::CopyAction);
        if (Qt::IgnoreAction == dragged) {
            event->accept();
            OnDetachTab(tabAt(m_dragStartPos), m_dragDropedPos);
        } else if (Qt::MoveAction == dragged) {
            if (!m_dragDropedPos.isNull()) {
                event->accept();
                this->OnMoveTab(tabAt(m_dragStartPos), tabAt(m_dragDropedPos));
            }
        }
        delete drag;
        drag = NULL;
    } else {
        QTabBar::mouseMoveEvent(event);
    }
}

//////////////////////////////////////////////////////////////////////////////
void MHTabBar::dragEnterEvent(QDragEnterEvent *event)
{
    // Only accept if it's an tab-reordering request
    const QMimeData *m = event->mimeData();
    QStringList formats = m->formats();
    if (formats.contains("action") && (m->data("action") == "application/tab-detach")) {
        event->acceptProposedAction();
    }
    QTabBar::dragEnterEvent(event);
}

//////////////////////////////////////////////////////////////////////////////
void MHTabBar::dragMoveEvent(QDragMoveEvent *event)
{
    // Only accept if it's an tab-reordering request
    const QMimeData *m = event->mimeData();
    QStringList formats = m->formats();
    if (formats.contains("action") && (m->data("action") == "application/tab-detach")) {
        m_dragMovedPos = event->pos();
        event->acceptProposedAction();
    }
    QTabBar::dragMoveEvent(event);
}

//////////////////////////////////////////////////////////////////////////////
void MHTabBar::dropEvent(QDropEvent *event)
{
    // If a dragged Event is dropped within this widget it is not a drag but
    // a move.
    m_dragDropedPos = event->pos();
    QTabBar::dropEvent(event);
}

//////////////////////////////////////////////////////////////////////////////
void MHTabBar::onRightClick(QPoint pos)
{
    QMenu myMenu(this);
    QAction *action;

    QPoint globalPos = this->mapToGlobal(pos);

    action = myMenu.addAction(tr("Detach Tab"));
    action->setToolTip(tr("You can put the tab back by closing the window"));
    action->setIcon(QIcon("://oxygen/32x32/actions/split.png"));

    QAction *selectedItem = myMenu.exec(globalPos);
    if (selectedItem == action) {
        OnDetachTab(tabAt(pos), m_dragDropedPos);
    }
}
