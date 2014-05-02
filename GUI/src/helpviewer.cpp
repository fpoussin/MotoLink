#include "helpviewer.h"
#include "ui_helpviewer.h"
#include <QFile>

HelpViewer::HelpViewer(QWidget *parent) :
    QWidget(parent),
    mUi(new Ui::HelpViewer)
{
    mUi->setupUi(this);
}

HelpViewer::~HelpViewer()
{
    delete mUi;
}

void HelpViewer::show()
{
    mUi->webView->load(QUrl("qrc:/doc/html/index.html"));
    QWidget::show();
}
