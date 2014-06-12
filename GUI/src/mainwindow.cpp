#include "mainwindow.h"
#include <QShortcut>
#include <QSignalMapper>
#include <ui_main.h>
#include <QFileInfo>
#include <QModelIndex>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mUi(new Ui::MainWindow),
    mSettings("Motolink", "Motolink"),
    mHelpViewer(NULL),
    mUndoStack(NULL),
    mFuelModel(&mUndoStack, -30, 30),
    mStagingModel(&mUndoStack, -30, 30),
    mAFRModel(&mUndoStack, 8, 22),
    mAFRTgtModel(&mUndoStack, 8, 22),
    mIgnModel(&mUndoStack, -20, 10),
    mKnockModel(&mUndoStack, 0, 256)
{
    mMtl = new Motolink();
    mHrc = new Hrc();
    mUpdateWizard = new UpdateWizard(mMtl);

    mUndoStack.setUndoLimit(100);

    mFuelModel.setName("Fuel");
    mStagingModel.setName("Staging");
    mAFRModel.setName("AFR");
    mAFRTgtModel.setName("AFR Target");
    mIgnModel.setName("Ignition");
    mKnockModel.setName("Knock");

    mUi->setupUi(this);
    this->setupDefaults();
    this->setupConnections();
    this->setupTabShortcuts();
    this->setupSettings();

    mUndoView.setStack(&mUndoStack);
    mUndoView.setWindowTitle("Actions History - "+this->windowTitle());
    mHasChanged = false;

    mUi->sbIdle->setName("Idle");
    mUi->sbPitLimiter->setName("Pit Limiter");
    mUi->sbShiftLight->setName("Shift Light");
    mUi->sbThresholdMin->setName("Minimum Threshold");
    mUi->sbThresholdMax->setName("Maximum Threshold");

    mUi->sbIdle->setUndoStack(&mUndoStack);
    mUi->sbPitLimiter->setUndoStack(&mUndoStack);
    mUi->sbShiftLight->setUndoStack(&mUndoStack);
    mUi->sbThresholdMin->setUndoStack(&mUndoStack);
    mUi->sbThresholdMax->setUndoStack(&mUndoStack);

    mSensorsTimer.setInterval(100);

    this->uiDisable();

    emit startupComplete();
}

MainWindow::~MainWindow()
{
    delete mUi;
    delete mUpdateWizard;
    delete mMtl;
    delete mHrc;

    for (int i = 0; i < MAX_RECENT_FILES; ++i)
    {
        if (mRecentFilesActions[i] != NULL)
            delete mRecentFilesActions[i];
    }
}

void MainWindow::Quit()
{
    if (mHasChanged) {

        int ret = QMessageBox::question(this, tr("Confirm"),
                    tr("Unsaved changes!\nSave file before closing?"),
                    QMessageBox::Save | QMessageBox::Discard
                    | QMessageBox::Cancel,
                    QMessageBox::Cancel);

        switch (ret) {
          case QMessageBox::Save:
              this->saveFile();
              break;
          case QMessageBox::Discard:
              break;
          case QMessageBox::Cancel:
              return;
        }
    }
    qApp->quit();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    this->Quit();
}

void MainWindow::openFile(void)
{
    QString fileName(QFileDialog::getOpenFileName(this,
                       tr("Open Tune File"), "", tr("Tune Files (*.xml)")));

    this->openFile(fileName);
}

void MainWindow::openFile(const QString &filename)
{
    if (filename.isEmpty())
        return;

    qWarning() << "Opening" << filename;

    QStringList files = mSettings.value(SETTINGS_RECENT_FILES).toStringList();

    mCurrentFile = filename;
    files.removeAll(filename);
    files.prepend(filename);
    while (files.size() > MAX_RECENT_FILES)
             files.removeLast();

    mSettings.setValue(SETTINGS_RECENT_FILES, files);
    this->updateRecentFilesActions();
}

void MainWindow::saveFile(void)
{
    if (mCurrentFile.length() == 0)
    {
        this->saveFileAs();
    }
}

void MainWindow::saveFileAs(void)
{
    QString fileName(QFileDialog::getSaveFileName(this,
                       tr("Save Tune File"), "", tr("Tune Files (*.xml)")));

    if (fileName.isEmpty())
    {
        return;
    }
    mCurrentFile = fileName;
}

void MainWindow::connectMtl()
{
    if (mMtl->usbConnect())
    {
        mMtl->bootAppIfNeeded();
        this->uiEnable();
        this->mSensorsTimer.start();
    }
    else {
        mUi->statusBar->showMessage(tr("Connection Failed"));
    }
}

void MainWindow::disconnectMtl()
{
    this->mSensorsTimer.stop();
    mMtl->usbDisconnect();
    this->uiDisable();
}

void MainWindow::showAbout()
{
    QMessageBox::information(this,tr("About Motolink"),
       tr("<strong>Version: " __MTL_VER__ "</strong><br/><br/>")+
       tr("Motolink is a smart interface designed for Honda HRC ECUs.<br/><br/>"
       "You can find more information "
       "<a href=\"https://github.com/fpoussin/MotoLink\">here.</a>"));
}

void MainWindow::showUpdateDialog()
{
    mUpdateWizard->showWizard();
}

void MainWindow::importHrc()
{
    QString fileName(QFileDialog::getOpenFileName(this,
                       tr("Import HRC File"), "", tr("HRC File (*.E2P)")));
    mHrc->openFile(fileName);
}

void MainWindow::exportHrc()
{
    QString fileName(QFileDialog::getSaveFileName(this,
                       tr("Export HRC File"), "", tr("HRC File (*.E2P)")));
    mHrc->saveFile(fileName);
}

void MainWindow::setupDefaults(void)
{
    mUi->statusBar->showMessage(tr("Disconnected"));

    mDegreeSuffix.setSuffix(QString::fromUtf8("°"));
    mPercentSuffix.setSuffix("%");

    mUi->tableFuel->setItemDelegate(&mPercentSuffix);
    mUi->tableIgnMap->setItemDelegate(&mDegreeSuffix);

    mUi->tableFuel->setModel(&mFuelModel);
    mUi->tableIgnMap->setModel(&mIgnModel);
    mUi->tableAfrMap->setModel(&mAFRModel);
    mUi->tableAfrTgt->setModel(&mAFRTgtModel);
    mUi->tableKnk->setModel(&mKnockModel);
}

void MainWindow::setupConnections(void)
{
    QObject::connect(mUi->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    QObject::connect(mUi->actionAbout_QT, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    QObject::connect(mUi->actionQuit, SIGNAL(triggered()), this, SLOT(Quit()));
    QObject::connect(mUi->actionOpen, SIGNAL(triggered()), this, SLOT(openFile()));
    QObject::connect(mUi->actionSave, SIGNAL(triggered()), this, SLOT(saveFile()));
    QObject::connect(mUi->actionSave_As, SIGNAL(triggered()), this, SLOT(saveFileAs()));
    QObject::connect(mUi->actionConnect, SIGNAL(triggered()), this, SLOT(connectMtl()));
    QObject::connect(mUi->actionDisconnect, SIGNAL(triggered()), this, SLOT(disconnectMtl()));
    QObject::connect(mUi->actionUpdate, SIGNAL(triggered()), this, SLOT(showUpdateDialog()));
    QObject::connect(mUi->actionImport, SIGNAL(triggered()), this, SLOT(importHrc()));
    QObject::connect(mUi->actionExport, SIGNAL(triggered()), this, SLOT(exportHrc()));
    QObject::connect(mUi->actionEnglish, SIGNAL(triggered()), this, SLOT(setLanguageEnglish()));
    QObject::connect(mUi->actionFran_ais, SIGNAL(triggered()), this, SLOT(setLanguageFrench()));
    QObject::connect(mUi->actionShowHelpIndex, SIGNAL(triggered()), this, SLOT(showHelp()));

    QObject::connect(mUi->actionShow_actions, SIGNAL(triggered()), &mUndoView, SLOT(show()));
    QObject::connect(mUi->actionUndo, SIGNAL(triggered()), &mUndoStack, SLOT(undo()));
    QObject::connect(mUi->actionRedo, SIGNAL(triggered()), &mUndoStack, SLOT(redo()));
    QObject::connect(&mUndoStack, SIGNAL(canRedoChanged(bool)), mUi->actionRedo, SLOT(setEnabled(bool)));
    QObject::connect(&mUndoStack, SIGNAL(canUndoChanged(bool)), mUi->actionUndo, SLOT(setEnabled(bool)));

    QObject::connect(&mFuelModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(showFuelTab()));
    QObject::connect(&mStagingModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(showStagingTab()));
    QObject::connect(&mAFRModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(showAFRTab()));
    QObject::connect(&mAFRTgtModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(showAFRTgtTab()));
    QObject::connect(&mIgnModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(showIgnTab()));
    QObject::connect(&mKnockModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(showKnockTab()));

    QObject::connect(mUi->sbIdle, SIGNAL(valueChanged(int)), this, SLOT(showSettingsTab()));
    QObject::connect(mUi->sbPitLimiter, SIGNAL(valueChanged(int)), this, SLOT(showSettingsTab()));
    QObject::connect(mUi->sbShiftLight, SIGNAL(valueChanged(int)), this, SLOT(showSettingsTab()));
    QObject::connect(mUi->sbThresholdMax, SIGNAL(valueChanged(int)), this, SLOT(showSettingsTab()));
    QObject::connect(mUi->sbThresholdMin, SIGNAL(valueChanged(int)), this, SLOT(showSettingsTab()));

    QObject::connect(this, SIGNAL(startupComplete()), &mUpdate, SLOT(getLatestVersion()));
    QObject::connect(&mUpdate, SIGNAL(newVersionAvailable(QString)), mUi->statusBar, SLOT(showMessage(QString)));

    mUi->tableAfrMap->setContextMenuPolicy(Qt::CustomContextMenu);
    mUi->tableAfrTgt->setContextMenuPolicy(Qt::CustomContextMenu);
    mUi->tableFuel->setContextMenuPolicy(Qt::CustomContextMenu);
    mUi->tableIgnMap->setContextMenuPolicy(Qt::CustomContextMenu);
    mUi->tableKnk->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(mUi->tableAfrMap, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showAfrMapContextMenu(QPoint)));
    QObject::connect(mUi->tableAfrTgt, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showAfrTgtContextMenu(QPoint)));
    QObject::connect(mUi->tableFuel, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showFuelContextMenu(QPoint)));
    QObject::connect(mUi->tableIgnMap, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showIgnContextMenu(QPoint)));
    QObject::connect(mUi->tableKnk, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showKnkContextMenu(QPoint)));

    for (int i = 0; i < MAX_RECENT_FILES; ++i) {
             mRecentFilesActions[i] = new QAction(this);
             mRecentFilesActions[i]->setVisible(false);
             mRecentFilesActions[i]->setIcon(QIcon("://oxygen/32x32/actions/quickopen-file.png"));
             connect(mRecentFilesActions[i], SIGNAL(triggered()),
                     this, SLOT(openRecenFile()));
             mUi->menuRecent_files->addAction(mRecentFilesActions[i]);
         }

    /* Sensors UI update */
    QObject::connect(&mSensorsTimer, SIGNAL(timeout()), this, SLOT(updateSensors()));
    QObject::connect(this, SIGNAL(sendUpdateSensors(QByteArray*)), mMtl, SLOT(getSensors(QByteArray*)));
    QObject::connect(mMtl, SIGNAL(sendSensors(QByteArray*)), this, SLOT(receiveSensors(QByteArray*)));
}

void MainWindow::setupTabShortcuts()
{
    QSignalMapper *signalMapper = new QSignalMapper(this);

    /* Assign tab shortcuts starting from F5 up to F12 */
    for(int index=0; index < mUi->tabMain->count(); ++index)
    {
       if (index > 7) break;
       QShortcut *shortcut = new QShortcut(
                   QKeySequence(QString("F%1").arg(index + 5)), this);

       QObject::connect(shortcut, SIGNAL( activated()),
                         signalMapper, SLOT(map()));
       signalMapper->setMapping(shortcut, index);
    }
    QObject::connect(signalMapper, SIGNAL(mapped(int)),
                     mUi->tabMain, SLOT(setCurrentIndex(int)));
}

void MainWindow::setupSettings()
{
    const QString lang = mSettings.value(SETTINGS_LANGUAGE, "English").toString();
    mRecentFiles = mSettings.value(SETTINGS_RECENT_FILES, QStringList()).toStringList();

    if (lang == "French") {
        this->setLanguageFrench();
    }
    else {
        this->setLanguageEnglish();
    }

    this->updateRecentFilesActions();

}

void MainWindow::retranslate()
{
    mUi->retranslateUi(this);
    mUpdateWizard->retranslate();
}

void MainWindow::setLanguageEnglish()
{
    qApp->removeTranslator(&mTranslator);
    this->retranslate();
    mSettings.setValue("main/language", "English");
}

void MainWindow::setLanguageFrench()
{
    if (!mTranslator.load(":/tr/motolink_fr")) {
        qWarning() << "Failed to load translation";
        return;
    }

    qApp->installTranslator(&mTranslator);
    this->retranslate();
    mSettings.setValue("main/language", "French");
}

void MainWindow::showHelp()
{
    mHelpViewer.show();
}

void MainWindow::uiEnable()
{
    const bool toggle = true;
    mUi->tabMain->setEnabled(toggle);

    mUi->actionSave->setEnabled(toggle);
    mUi->actionSave_As->setEnabled(toggle);
    mUi->actionImport->setEnabled(toggle);
    mUi->actionExport->setEnabled(toggle);
    mUi->actionConnect->setEnabled(!toggle);
    mUi->actionDisconnect->setEnabled(toggle);
    mUi->actionGet_Configuration->setEnabled(toggle);
    mUi->actionSend_Configuration->setEnabled(toggle);
    mUi->actionAuto_Send->setEnabled(toggle);
}

void MainWindow::uiDisable()
{
    const bool toggle = false;
    mUi->tabMain->setEnabled(toggle);

    mUi->actionSave->setEnabled(toggle);
    mUi->actionSave_As->setEnabled(toggle);
    mUi->actionImport->setEnabled(toggle);
    mUi->actionExport->setEnabled(toggle);
    mUi->actionConnect->setEnabled(!toggle);
    mUi->actionDisconnect->setEnabled(toggle);
    mUi->actionGet_Configuration->setEnabled(toggle);
    mUi->actionSend_Configuration->setEnabled(toggle);
    mUi->actionAuto_Send->setEnabled(toggle);
}

void MainWindow::updateRecentFilesActions()
{
    QStringList files = mSettings.value(SETTINGS_RECENT_FILES).toStringList();
    const uint numRecentFiles = qMin(files.size(), (int)MAX_RECENT_FILES);

    for (uint i = 0; i < numRecentFiles; ++i)
    {
        QString text = QFileInfo(files[i]).fileName();
        mRecentFilesActions[i]->setText(text);
        mRecentFilesActions[i]->setData(files[i]);
        mRecentFilesActions[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MAX_RECENT_FILES; ++j)
        mRecentFilesActions[j]->setVisible(false);
}

void MainWindow::openRecenFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        this->openFile(action->data().toString());
}

void MainWindow::showFuelTab()
{
    const int index = mUi->tabMain->indexOf(mUi->tabFuel);

    if (index >= 0)
        mUi->tabMain->setCurrentIndex(index);
}

void MainWindow::showAFRTab()
{
    const int index = mUi->tabMain->indexOf(mUi->tabAfrMap);

    if (index >= 0)
        mUi->tabMain->setCurrentIndex(index);
}

void MainWindow::showStagingTab()
{
    const int index = mUi->tabMain->indexOf(mUi->tabStaging);

    if (index >= 0)
        mUi->tabMain->setCurrentIndex(index);
}

void MainWindow::showAFRTgtTab()
{
    const int index = mUi->tabMain->indexOf(mUi->tabAfrTarget);

    if (index >= 0)
        mUi->tabMain->setCurrentIndex(index);
}

void MainWindow::showIgnTab()
{
    const int index = mUi->tabMain->indexOf(mUi->tabIgnMap);

    if (index >= 0)
        mUi->tabMain->setCurrentIndex(index);
}

void MainWindow::showKnockTab()
{
    const int index = mUi->tabMain->indexOf(mUi->tabKnock);

    if (index >= 0)
        mUi->tabMain->setCurrentIndex(index);
}

void MainWindow::showSettingsTab()
{
    const int index = mUi->tabMain->indexOf(mUi->tabSettings);

    if (index >= 0)
        mUi->tabMain->setCurrentIndex(index);
}

void MainWindow::showAfrMapContextMenu(const QPoint &pos)
{
    this->showDefaultContextMenu(pos, mUi->tableAfrMap);
}

void MainWindow::showAfrTgtContextMenu(const QPoint &pos)
{
    this->showDefaultContextMenu(pos, mUi->tableAfrTgt);
}

void MainWindow::showFuelContextMenu(const QPoint &pos)
{
    this->showDefaultContextMenu(pos, mUi->tableFuel);
}

void MainWindow::showIgnContextMenu(const QPoint &pos)
{
    this->showDefaultContextMenu(pos, mUi->tableIgnMap);
}

void MainWindow::showKnkContextMenu(const QPoint &pos)
{
    this->showDefaultContextMenu(pos, mUi->tableKnk);
}

void MainWindow::showDefaultContextMenu(const QPoint &pos, QTableView *view)
{
    QPoint globalPos = view->viewport()->mapToGlobal(pos);
    /* TODO */

    QMenu myMenu;
    myMenu.addAction("Increase selection");
    myMenu.addSeparator();
    myMenu.addAction("Decrease selection");

    QAction* selectedItem = myMenu.exec(globalPos);

    if (selectedItem)
    {
        // something was chosen, do stuff
    }
    else
    {
        // nothing was chosen
    }
}

void MainWindow::updateSensors()
{
    if (mMtl->isConnected())
        emit sendUpdateSensors(&mSensorsData);
    else
        mSensorsTimer.stop();
}

void MainWindow::receiveSensors(QByteArray *data)
{
    const sensors_t * sensors =  (sensors_t *)data->constData();

    float vAn7 = sensors->an7/1000.0; /* VBAT */
    float vAn8 = sensors->an8/1000.0; /* TPS */
    float vAn9 = sensors->an9/1000.0; /* AFR */
    mUi->lVbat->setText(QString::number(vAn7)+tr(" Volts"));
    mUi->lTpsVolts->setText(QString::number(vAn8)+tr(" Volts"));
    mUi->lAfrVolts->setText(QString::number(vAn9)+tr(" Volts"));
    mUi->lRpmHertz->setText(QString::number(sensors->freq1)+tr(" Hertz"));
    mUi->lSpeedHertz->setText(QString::number(sensors->freq2)+tr(" Hertz"));
}

