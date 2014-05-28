#include "updatewizard.h"
#include "ui_updatewizard.h"
#include <QtXml>

UpdateWizard::UpdateWizard(Bootloader *btl, Motolink *mtl, QWidget *parent) :
    QWizard(parent),
    mUi(new Ui::UpdateWizard)
{
    mUi->setupUi(this);
    this->setOptions(QWizard::DisabledBackButtonOnLastPage
                     | QWizard::NoBackButtonOnStartPage);
    mBtl = btl;
    mMtl = mtl;
    mTft = new TransferThread(btl);
    this->setupConnections();
}

UpdateWizard::~UpdateWizard()
{
    delete mTft;
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

void UpdateWizard::updateStatus(QString text)
{
    mUi->lStatus->setText(mUi->lStatus->text()+QString("\n")+text);
}

void UpdateWizard::disableButtons(bool disable)
{

}

void UpdateWizard::setupConnections()
{
    QObject::connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(pageUpdated(int)));
    QObject::connect(mTft, SIGNAL(sendStatus(QString)), this, SLOT(updateStatus(QString)));
    QObject::connect(mTft, SIGNAL(sendProgress(int)), this->mUi->pbProgress, SLOT(setValue(int)));
    QObject::connect(mTft, SIGNAL(sendLock(bool)), this, SLOT(disableButtons(bool)));

    QObject::connect(this, SIGNAL(send(QByteArray*)), mTft, SLOT(send(QByteArray*)));
    QObject::connect(this, SIGNAL(verify(QByteArray*)), mTft, SLOT(verify(QByteArray*)));

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
            }
        }
        n = n.nextSibling();
    }
}

void UpdateWizard::connectBtl()
{
    mMtl->disconnect();

    if (mBtl->connect())
    {
        mUi->lStatus->clear();
        this->updateStatus(tr("Connected"));
        mUi->pbProgress->setValue(10);

        mTft->setParams(mBtl, &mFwData, true, true);
        mTft->run();
    }

    if (mBtl->disconnect())
    {
        this->updateStatus(tr("Disconnected"));
    }
}
