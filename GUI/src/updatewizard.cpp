#include "updatewizard.h"
#include "ui_updatewizard.h"
#include <QtXml>

UpdateWizard::UpdateWizard(Bootloader *btl, QWidget *parent) :
    QWizard(parent),
    mUi(new Ui::UpdateWizard)
{
    mUi->setupUi(this);
    this->setOptions(QWizard::DisabledBackButtonOnLastPage
                     | QWizard::NoBackButtonOnStartPage);
    mBtl = btl;
    this->setupConnections();
}

UpdateWizard::~UpdateWizard()
{
    delete mUi;
}

void UpdateWizard::showWizard()
{
    this->loadFirmareData();
    this->mUi->lNewVersion->setText(mNewVersion);
    this->show();
}

void UpdateWizard::startUpdate()
{
    mUi->pbProgress->setValue(50);
}

void UpdateWizard::retranslate()
{
    mUi->retranslateUi(this);
}

void UpdateWizard::pageUpdated(int page)
{
    switch (page)
    {
        case 0:
            break;
        case 1:
            this->connectBtl();
            break;
        default:
            break;
    }

}

void UpdateWizard::setupConnections()
{
    QObject::connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(pageUpdated(int)));
}

void UpdateWizard::loadFirmareData()
{
    QDomDocument doc("firmware");
    QFile file(":/fw/firmware.xml");

    if (!file.open(QIODevice::ReadOnly))
        return;

    if (!doc.setContent(&file))
    {
        file.close();
        return;
    }
    file.close();

    QDomElement docElem = doc.documentElement();
    QDomNode n = docElem.firstChild();

    while(!n.isNull())
    {
        QDomElement e = n.toElement();
        qDebug() << qPrintable(e.tagName());
        if (n.childNodes().count() > 1)
        {
            QDomNode cn = n.firstChild();
            while (!cn.isNull())
            {
                QDomElement ce = cn.toElement();
                if(!ce.isNull())
                {
                    if (ce.tagName() == "version")
                    {
                        this->mNewVersion = ce.text();
                    }
                    //qDebug() << qPrintable(ce.tagName()) << qPrintable(ce.text());
                }
                cn = cn.nextSibling();
            }
        }
        else
        {
            if(!e.isNull())
            {
                if (e.tagName() == "data")
                {
                    this->mFwData = QByteArray::fromBase64(e.text().toLocal8Bit());
                }
                //qDebug() << qPrintable(e.text());
            }
        }
        n = n.nextSibling();
    }
}

void UpdateWizard::connectBtl()
{
    this->mMtl->disconnect();
    this->mBtl->connect();
}
