#include "updatewizard.h"
#include "ui_updatewizard.h"
#include <QtXml>

UpdateWizard::UpdateWizard(Motolink * const mtl, QWidget *parent) :
    QWizard(parent),
    mUi(new Ui::UpdateWizard),
    mMtl(mtl)
{
    mUi->setupUi(this);
    this->setOptions(QWizard::NoBackButtonOnStartPage);
    this->setupConnections();

    msError = tr("Error: ");
}

UpdateWizard::~UpdateWizard()
{
    delete mUi;
}

void UpdateWizard::showWizard()
{
    this->loadDefaultFirmareData();
    mUi->lNewVersion->setText(mNewVersion);
    this->enableButtons();
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

void UpdateWizard::openCustomFw()
{
    QString fileName(QFileDialog::getOpenFileName(this,
                       tr("Open firmware file"), "", tr("Firmware files (*.bin)")));

    if (fileName.isEmpty())
    {
        mUi->rbBundledFW->setChecked(true);
        this->loadDefaultFirmareData();
        return;
    }

    QFile fwFile(fileName);

    if(!fwFile.open(QIODevice::ReadOnly))
    {
        qWarning() << msError << tr("Couldn't open ") << fileName;
        fwFile.close();
        return;
    }

    mFwData = fwFile.readAll();
    mNewVersion = "Custom";
    mUi->lNewVersion->setText(mNewVersion);

    fwFile.close();
}

void UpdateWizard::pageUpdated(int page)
{
    switch (page)
    {
        case 0:
            break;
        case 1:
            this->startFwUpdate();
            break;
        case 2:
            break;
        default:
            break;
    }
}

void UpdateWizard::updateStatus(QString text)
{
    mUi->tbStatus->append(text);
}

void UpdateWizard::enableButtons(void)
{
    this->button(QWizard::CancelButton)->setEnabled(true);
    this->button(QWizard::NextButton)->setEnabled(true);
    this->button(QWizard::FinishButton)->setEnabled(true);
    this->button(QWizard::BackButton)->setEnabled(true);
}

void UpdateWizard::disableButtons(void)
{
    this->button(QWizard::CancelButton)->setEnabled(false);
    this->button(QWizard::NextButton)->setEnabled(false);
    this->button(QWizard::FinishButton)->setEnabled(false);
    this->button(QWizard::BackButton)->setEnabled(false);
}

void UpdateWizard::updateDone()
{
    this->enableButtons();
    emit sendStatus(tr("Booting app"));
    mMtl->getBtl()->boot();
}

void UpdateWizard::setupConnections()
{
    QObject::connect(mMtl->getTft(), SIGNAL(sendStatus(QString)), this, SLOT(updateStatus(QString)));
    QObject::connect(mMtl->getTft(), SIGNAL(sendProgress(int)), mUi->pbProgress, SLOT(setValue(int)));
    QObject::connect(mMtl->getTft(), SIGNAL(finished()), this, SLOT(updateDone()));

    QObject::connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(pageUpdated(int)));
    QObject::connect(this, SIGNAL(startTransfer()), mMtl->getTft(), SLOT(start()));
    QObject::connect(this, SIGNAL(send(QByteArray*)), mMtl->getTft(), SLOT(send(QByteArray*)));
    QObject::connect(this, SIGNAL(verify(QByteArray*)), mMtl->getTft(), SLOT(verify(QByteArray*)));
    QObject::connect(this, SIGNAL(sendStatus(QString)), this, SLOT(updateStatus(QString)));

    QObject::connect(mUi->rbCustomFW, SIGNAL(clicked()), this, SLOT(openCustomFw()));
    QObject::connect(mUi->rbBundledFW, SIGNAL(clicked()), this, SLOT(loadDefaultFirmareData()));
}

void UpdateWizard::loadDefaultFirmareData()
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
                        mNewVersion = ce.text();
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
                    mFwData = QByteArray::fromBase64(e.text().toLocal8Bit());
                }
            }
        }
        n = n.nextSibling();
    }

    mUi->lNewVersion->setText(mNewVersion);
}

void UpdateWizard::startFwUpdate()
{
    mUi->tbStatus->clear();
    this->disableButtons();

    if (mMtl->usbConnect())
    {
        if (mMtl->getMode() != MODE_BL)
        {
            /* App mode, reset to bootloader */
            emit sendStatus(tr("Reset to Bootloader..."));

            QCoreApplication::processEvents();
            mMtl->resetDevice();
            mMtl->usbDisconnect();

            /* Wait for windows to detect the bootloader */
            for (int i = 0; i <= 20; i++)
            {
                 _usleep(100000);
                 mUi->pbProgress->setValue(i*5);
                 QCoreApplication::processEvents();
            }
            mMtl->probeConnect();

            if (mMtl->getMode() != MODE_BL)
            {
                emit sendStatus(tr("Failed to reset device"));
                return;
            }

        }
        emit sendStatus(tr("Connected"));
        mUi->pbProgress->setValue(10);

        const quint8 flags = mMtl->getBtl()->getFlags();

        if (flags & FLAG_IWDRST)
            emit sendStatus(tr("Watchdog reset"));

        if (flags & FLAG_SFTRST)
            emit sendStatus(tr("Software reset"));

        if (flags & FLAG_NOAPP)
            emit sendStatus(tr("No valid user application"));

        if (flags & FLAG_WAKE)
            emit sendStatus(tr("Bootloader Wakeup"));

        mMtl->getTft()->setParams(&mFwData, true, true);
        emit startTransfer();
    }
    else
    {
        emit sendStatus(tr("Connection Failed"));
        this->enableButtons();
        return;
    }
}
