// Qt includes
#include <QDialog>
#include <QEvent>
#include <QLabel>
#include <QLayout>
#include <QTabBar>
#include <QTableWidget>
#include <QBoxLayout>
#include <QPixmap>
#include "mhtabbar.h"
#include "mhtabwidget.h"

//////////////////////////////////////////////////////////////
void MHTabWidget::Initialize(QMainWindow *mainWindow)
{
    m_tabBar->Initialize(mainWindow);
}

//////////////////////////////////////////////////////////////
void MHTabWidget::ShutDown(void)
{
}

//////////////////////////////////////////////////////////////
// Default Constructor
//////////////////////////////////////////////////////////////
MHTabWidget::MHTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    m_tabBar = new MHTabBar(this);
    connect(m_tabBar, SIGNAL(OnDetachTab(int, QPoint &)), this, SLOT(DetachTab(int, QPoint &)));
    connect(m_tabBar, SIGNAL(OnMoveTab(int, int)), this, SLOT(MoveTab(int, int)));

    setTabBar(m_tabBar);
    setMovable(true);
}

//////////////////////////////////////////////////////////////
// Default Destructor
//////////////////////////////////////////////////////////////
MHTabWidget::~MHTabWidget(void)
{
    disconnect(m_tabBar, SIGNAL(OnMoveTab(int, int)), this, SLOT(MoveTab(int, int)));
    disconnect(m_tabBar, SIGNAL(OnDetachTab(int, QPoint &)), this, SLOT(DetachTab(int, QPoint &)));
}

//////////////////////////////////////////////////////////////////////////////
void MHTabWidget::MoveTab(int fromIndex, int toIndex)
{
    QWidget *w = widget(fromIndex);
    QIcon icon = tabIcon(fromIndex);
    QString text = tabText(fromIndex);

    removeTab(fromIndex);
    insertTab(toIndex, w, icon, text);
    setCurrentIndex(toIndex);
}

//////////////////////////////////////////////////////////////////////////////
void MHTabWidget::DetachTab(int index, QPoint &dropPoint)
{
    (void)dropPoint;
    // Create Window
    MHDetachedWindow *detachedWidget = new MHDetachedWindow(parentWidget());
    detachedWidget->setWindowModality(Qt::NonModal);
    // With layouter
    QVBoxLayout *mainLayout = new QVBoxLayout(detachedWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Find Widget and connect
    MHWorkflowWidget *tearOffWidget = dynamic_cast<MHWorkflowWidget *>(widget(index));
    QObject::connect(detachedWidget, SIGNAL(OnClose(QWidget *)), this, SLOT(AttachTab(QWidget *)));
    detachedWidget->setWindowTitle(tabText(index));
    detachedWidget->setTabIcon(this->tabIcon(index));
    // Remove from tab bar
    tearOffWidget->setParent(detachedWidget);

    // Make first active
    if (0 < count()) {
        this->setCurrentIndex(0);
    }

    // Create and show
    mainLayout->addWidget(tearOffWidget);
    // Needs to be done explicit
    tearOffWidget->show();
    //detachedWidget->move (dropPoint);
    //    detachedWidget->setFixedSize(tearOffWidget->size());
    //    detachedWidget->setSizeGripEnabled(false);
    detachedWidget->show();
}

//////////////////////////////////////////////////////////////////////////////
void MHTabWidget::AttachTab(QWidget *parent)
{
    // Retrieve widget
    MHDetachedWindow *detachedWidget = dynamic_cast<MHDetachedWindow *>(parent);
    MHWorkflowWidget *tearOffWidget = dynamic_cast<MHWorkflowWidget *>(detachedWidget->layout()->takeAt(0)->widget());

    // Change parent
    tearOffWidget->setParent(this);

    // Attach
    int newIndex = addTab(tearOffWidget, detachedWidget->windowTitle());

    // Make Active
    if (-1 != newIndex) {
        this->setCurrentIndex(newIndex);
    }

    this->setTabIcon(this->currentIndex(), detachedWidget->tabIcon());
    // Cleanup Window
    QObject::disconnect(detachedWidget, SIGNAL(OnClose(QWidget *)), this, SLOT(AttachTab(QWidget *)));
    delete detachedWidget;
}

//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
// Default constructor
//////////////////////////////////////////////////////////////////////////////
MHWorkflowWidget::MHWorkflowWidget(QWidget *parent)
    : QWidget(parent)
{
}

//////////////////////////////////////////////////////////////////////////////
// Default destructor
//////////////////////////////////////////////////////////////////////////////
MHWorkflowWidget::~MHWorkflowWidget(void)
{
}

//----------------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////////
MHDetachedWindow::MHDetachedWindow(QWidget *parent)
    : QDialog(parent)
{
}

//////////////////////////////////////////////////////////////////////////////
MHDetachedWindow::~MHDetachedWindow(void)
{
}

void MHDetachedWindow::setTabIcon(const QIcon &icon)
{
    m_tabIcon = icon;
}

//////////////////////////////////////////////////////////////////////////////
QIcon MHDetachedWindow::tabIcon()
{
    return m_tabIcon;
}

//////////////////////////////////////////////////////////////////////////////
void MHDetachedWindow::closeEvent(QCloseEvent * /*event*/)
{
    emit OnClose(this);
}
