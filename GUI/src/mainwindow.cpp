#include "mainwindow.h"
#include <QShortcut>
#include <QSignalMapper>
#include <ui_main.h>
#include <ui_tasks.h>
#include <ui_knock.h>
#include <QFileInfo>
#include <QModelIndex>
#include <QDateTime>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mMainUi(new Ui::MainWindow),
    mTasksUi(new Ui::Tasks),
    mKnockGraphUi(new Ui::KnockGraph),
    mTasksWidget(new QWidget),
    mKnockGraphWidget(new QWidget),
    mSettings("Motolink", "Motolink"),
    mHelpViewer(NULL),
    mUndoStack(NULL),
    mFuelModel(&mUndoStack, -30, 30, 0),
    mStagingModel(&mUndoStack, -30, 30, 0),
    mAFRModel(&mUndoStack, 80, 200, 130),
    mAFRTgtModel(&mUndoStack, 80, 200, 130),
    mIgnModel(&mUndoStack, -20, 3, 0),
    mKnockModel(&mUndoStack, 0, 255, 0)
{
    mMtl = new Motolink();
    mHrc = new Hrc();
    mUpdateWizard = new UpdateWizard(mMtl);

    mUndoStack.setUndoLimit(100);

    mMainUi->setupUi(this);
    mTasksUi->setupUi(mTasksWidget);
    mKnockGraphUi->setupUi(mKnockGraphWidget);

    this->setupDefaults();
    this->setupConnections();
    this->setupTabShortcuts();
    this->setupSettings();
    this->setupKnockGraph();

    mUndoView.setStack(&mUndoStack);
    mUndoView.setWindowTitle(tr("Actions History - ")+this->windowTitle());
    mHasChanged = false;

    mMainUi->sbIdle->setName(tr("Idle"));
    mMainUi->sbPitLimiter->setName(tr("Pit Limiter"));
    mMainUi->sbShiftLight->setName(tr("Shift Light"));
    mMainUi->sbThresholdMin->setName(tr("Minimum Threshold"));
    mMainUi->sbThresholdMax->setName(tr("Maximum Threshold"));

    mMainUi->sbIdle->setUndoStack(&mUndoStack);
    mMainUi->sbPitLimiter->setUndoStack(&mUndoStack);
    mMainUi->sbShiftLight->setUndoStack(&mUndoStack);
    mMainUi->sbThresholdMin->setUndoStack(&mUndoStack);
    mMainUi->sbThresholdMax->setUndoStack(&mUndoStack);

    mFastPollingTimer.setInterval(50);
    mSlowPollingTimer.setInterval(500);

    this->exportToMTLFile();
    this->uiDisable();

    emit signalStartupComplete();
}

MainWindow::~MainWindow()
{
    delete mMainUi;
    delete mUpdateWizard;
    delete mTasksUi;
    delete mTasksWidget;
    delete mKnockGraphUi;
    delete mKnockGraphWidget;
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

    QFile file(mCurrentFile);

    if (!file.open(QFile::ReadOnly))
    {
        mMainUi->statusBar->showMessage(
                    tr("Failed to open file for reading!"));
        return;
    }

    mFile.read(&file);
    file.close();

    this->importFromMTLFile();
}

void MainWindow::saveFile(void)
{
    if (mCurrentFile.length() == 0)
    {
        this->saveFileAs();
        return;
    }

    QFile file(mCurrentFile);

    qWarning() << "Saving" << mCurrentFile;
    if (!file.open(QFile::WriteOnly))
    {
        mMainUi->statusBar->showMessage(
                    tr("Failed to open file for writing!"));
        qWarning() << tr("Failed to open file for writing!") << mCurrentFile;
        return;
    }

    file.seek(0);
    this->exportToMTLFile();
    mFile.write(&file);
    file.close();
    mMainUi->statusBar->showMessage(
                tr("File saved."));

    mHasChanged = false;
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

    this->saveFile();
}

void MainWindow::connectMtl()
{
    if (mMtl->usbConnect())
    {
        mMtl->bootAppIfNeeded();
        this->uiEnable();
        mFastPollingTimer.start();
        mSlowPollingTimer.start();
        mMainUi->statusBar->showMessage("Connected");
    }
    else {
        mMainUi->statusBar->showMessage(tr("Connection Failed"));
    }
}

void MainWindow::disconnectMtl()
{
    mFastPollingTimer.stop();
    mSlowPollingTimer.stop();
    mMtl->usbDisconnect();
    this->uiDisable();
    mMainUi->statusBar->showMessage("Disconnected");
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
    mMainUi->statusBar->showMessage(tr("Disconnected"));

    mTablesModelList.append(&mFuelModel);
    mTablesModelList.append(&mStagingModel);
    mTablesModelList.append(&mAFRModel);
    mTablesModelList.append(&mAFRTgtModel);
    mTablesModelList.append(&mIgnModel);
    mTablesModelList.append(&mKnockModel);

    mTablesViewList.append(mMainUi->tableAfrMap);
    mTablesViewList.append(mMainUi->tableAfrTgt);
    mTablesViewList.append(mMainUi->tableFuel);
    mTablesViewList.append(mMainUi->tableIgnMap);
    mTablesViewList.append(mMainUi->tableKnk);

    mSpinBoxList.append(mMainUi->sbIdle);
    mSpinBoxList.append(mMainUi->sbPitLimiter);
    mSpinBoxList.append(mMainUi->sbShiftLight);
    mSpinBoxList.append(mMainUi->sbThresholdMax);
    mSpinBoxList.append(mMainUi->sbThresholdMin);

    mFuelModel.setName(tr("Fuel"));
    mStagingModel.setName(tr("Staging"));
    mAFRModel.setName(tr("AFR"));
    mAFRTgtModel.setName(tr("AFR Target"));
    mIgnModel.setName(tr("Ignition"));
    mKnockModel.setName(tr("Knock"));

    mDegreeSuffix.setSuffix(QString::fromUtf8("°"));
    mPercentSuffix.setSuffix("%");

    mMainUi->tableFuel->setItemDelegate(&mPercentSuffix);
    mMainUi->tableIgnMap->setItemDelegate(&mDegreeSuffix);
    mMainUi->tableAfrMap->setItemDelegate(&mAfrDisplay);
    mMainUi->tableAfrTgt->setItemDelegate(&mAfrDisplay);

    mMainUi->tableFuel->setModel(&mFuelModel);
    mMainUi->tableIgnMap->setModel(&mIgnModel);
    mMainUi->tableAfrMap->setModel(&mAFRModel);
    mMainUi->tableAfrTgt->setModel(&mAFRTgtModel);
    mMainUi->tableKnk->setModel(&mKnockModel);

}

void MainWindow::setupConnections(void)
{
    QObject::connect(mMainUi->actionAbout, SIGNAL(triggered()), this, SLOT(showAbout()));
    QObject::connect(mMainUi->actionAbout_QT, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    QObject::connect(mMainUi->actionQuit, SIGNAL(triggered()), this, SLOT(Quit()));
    QObject::connect(mMainUi->actionOpen, SIGNAL(triggered()), this, SLOT(openFile()));
    QObject::connect(mMainUi->actionSave, SIGNAL(triggered()), this, SLOT(saveFile()));
    QObject::connect(mMainUi->actionSave_As, SIGNAL(triggered()), this, SLOT(saveFileAs()));
    QObject::connect(mMainUi->actionConnect, SIGNAL(triggered()), this, SLOT(connectMtl()));
    QObject::connect(mMainUi->actionDisconnect, SIGNAL(triggered()), this, SLOT(disconnectMtl()));
    QObject::connect(mMainUi->actionUpdate, SIGNAL(triggered()), this, SLOT(showUpdateDialog()));
    QObject::connect(mMainUi->actionImport, SIGNAL(triggered()), this, SLOT(importHrc()));
    QObject::connect(mMainUi->actionExport, SIGNAL(triggered()), this, SLOT(exportHrc()));
    QObject::connect(mMainUi->actionEnglish, SIGNAL(triggered()), this, SLOT(setLanguageEnglish()));
    QObject::connect(mMainUi->actionFran_ais, SIGNAL(triggered()), this, SLOT(setLanguageFrench()));
    QObject::connect(mMainUi->actionShowHelpIndex, SIGNAL(triggered()), this, SLOT(showHelp()));
    QObject::connect(mMainUi->actionShow_tasks, SIGNAL(triggered()),this, SLOT(showTasks()));
    QObject::connect(mMainUi->actionShow_Knock_Spectrum, SIGNAL(triggered()),this, SLOT(showKnockGraph()));

    QObject::connect(mMainUi->actionShow_actions, SIGNAL(triggered()), &mUndoView, SLOT(show()));
    QObject::connect(mMainUi->actionUndo, SIGNAL(triggered()), &mUndoStack, SLOT(undo()));
    QObject::connect(mMainUi->actionRedo, SIGNAL(triggered()), &mUndoStack, SLOT(redo()));
    QObject::connect(&mUndoStack, SIGNAL(canRedoChanged(bool)), mMainUi->actionRedo, SLOT(setEnabled(bool)));
    QObject::connect(&mUndoStack, SIGNAL(canUndoChanged(bool)), mMainUi->actionUndo, SLOT(setEnabled(bool)));

    for (int i=0; i<mTablesModelList.size(); i++)
    {
        TableModel* tbl = mTablesModelList.at(i);
        QObject::connect(tbl, SIGNAL(cellValueChanged()), this, SLOT(onDataChanged()));
        QObject::connect(tbl, SIGNAL(headerDataNeedSync(int,Qt::Orientation,QVariant)),
                         this, SLOT(onHeaderDataNeedSync(int,Qt::Orientation,QVariant)));
        QObject::connect(tbl->view(), SIGNAL(modelUpdated(QWidget*)), mMainUi->tabMain, SLOT(setCurrentWidget(QWidget*)));

    }

    for (int i=0; i<mSpinBoxList.size(); i++)
    {
        QSpinBox * sb = mSpinBoxList.at(i);
        QObject::connect(sb, SIGNAL(valueChanged(int)), this, SLOT(showSettingsTab()));
        QObject::connect(sb, SIGNAL(valueChanged(int)), this, SLOT(onDataChanged()));
    }

    QObject::connect(this, SIGNAL(signalStartupComplete()), &mUpdate, SLOT(getLatestVersion()));
    QObject::connect(&mUpdate, SIGNAL(newVersionAvailable(QString)), this, SLOT(showNewVersionPopup(QString)));
    QObject::connect(mUpdateWizard, SIGNAL(sendDisconnect()), this, SLOT(disconnectMtl()));

    for (int i=0; i<mTablesViewList.size(); i++)
    {
        QEnhancedTableView* tbl = mTablesViewList.at(i);
        tbl->setContextMenuPolicy(Qt::CustomContextMenu);
        QObject::connect(tbl, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showDefaultContextMenu(QPoint)));
    }

    QObject::connect(mMainUi->bTpsSet0, SIGNAL(clicked()), this, SLOT(onSetTps0Pct()));
    QObject::connect(mMainUi->bTpsSet100, SIGNAL(clicked()), this, SLOT(onSetTps100Pct()));

    for (int i = 0; i < MAX_RECENT_FILES; ++i) {
             mRecentFilesActions[i] = new QAction(this);
             mRecentFilesActions[i]->setVisible(false);
             mRecentFilesActions[i]->setIcon(QIcon("://oxygen/32x32/actions/quickopen-file.png"));
             connect(mRecentFilesActions[i], SIGNAL(triggered()),
                     this, SLOT(openRecenFile()));
             mMainUi->menuRecent_files->addAction(mRecentFilesActions[i]);
         }

    /* Sensors UI update */
    QObject::connect(&mFastPollingTimer, SIGNAL(timeout()), this, SLOT(doFastPolling()));
    QObject::connect(&mSlowPollingTimer, SIGNAL(timeout()), this, SLOT(doSlowPolling()));

    QObject::connect(this, SIGNAL(signalRequestSensors(QByteArray*)), mMtl, SLOT(getSensors(QByteArray*)));
    QObject::connect(mMtl, SIGNAL(sendSensors(QByteArray*)), this, SLOT(onSensorsDataReceived(QByteArray*)));

    QObject::connect(this, SIGNAL(signalRequestMonitoring(QByteArray*)), mMtl, SLOT(getMonitoring(QByteArray*)));
    QObject::connect(mMtl, SIGNAL(sendMonitoring(QByteArray*)), this, SLOT(onMonitoringDataReceived(QByteArray*)));

    QObject::connect(this, SIGNAL(signalRequestKnock(QByteArray*)), mMtl, SLOT(getKnockSpectrum(QByteArray*)));
    QObject::connect(mMtl, SIGNAL(sendKockSpectrum(QByteArray*)), this, SLOT(OnKnockSpectrumDataReceived(QByteArray*)));

    QObject::connect(&mFile, SIGNAL(readFailed(QString)), this, SLOT(onSimpleError(QString)));
}

void MainWindow::setupTabShortcuts()
{
    QSignalMapper *signalMapper = new QSignalMapper(this);

    /* Assign tab shortcuts starting from F5 up to F12 */
    for(int index=0; index < mMainUi->tabMain->count(); ++index)
    {
       if (index > 7) break;
       QShortcut *shortcut = new QShortcut(
                   QKeySequence(QString("F%1").arg(index + 5)), this);

       QObject::connect(shortcut, SIGNAL( activated()),
                         signalMapper, SLOT(map()));
       signalMapper->setMapping(shortcut, index);
    }
    QObject::connect(signalMapper, SIGNAL(mapped(int)),
                     mMainUi->tabMain, SLOT(setCurrentIndex(int)));
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

void MainWindow::setupKnockGraph()
{
    QCustomPlot * plot = mKnockGraphUi->mainPlot;
    mKnockFreqLabel = new QCPItemText(plot);

    plot->addGraph();
    plot->xAxis->setLabel(tr("Knock Frequency (Hertz)"));
    plot->yAxis->setLabel(tr("Knock Intensity (Volts, AC)"));
    plot->xAxis->setRange(0, FFT_FREQ/2);
    plot->yAxis->setRange(0, KNOCK_MAX);

    plot->addItem(mKnockFreqLabel);
    mKnockFreqLabel->setPositionAlignment(Qt::AlignTop|Qt::AlignHCenter);
    mKnockFreqLabel->position->setType(QCPItemPosition::ptAxisRectRatio);
    mKnockFreqLabel->position->setCoords(0.5, 0); // place position at center/top of axis rect
    mKnockFreqLabel->setText(tr("Loading..."));
    mKnockFreqLabel->setFont(QFont(font().family(), 16)); // make font a bit larger
    mKnockFreqLabel->setPen(QPen(Qt::black)); // show black border around text
}

void MainWindow::retranslate()
{
    mMainUi->retranslateUi(this);
    mUpdateWizard->retranslate();
    mKnockGraphUi->retranslateUi(mKnockGraphWidget);
    mTasksUi->retranslateUi(mTasksWidget);

    mMainUi->tableFuel->retranslate();
    mMainUi->tableAfrMap->retranslate();
    mMainUi->tableAfrTgt->retranslate();
    mMainUi->tableIgnMap->retranslate();
    mMainUi->tableKnk->retranslate();
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

    mMainUi->actionConnect->setEnabled(!toggle);
    mMainUi->actionDisconnect->setEnabled(toggle);
    mMainUi->actionGet_Configuration->setEnabled(toggle);
    mMainUi->actionSend_Configuration->setEnabled(toggle);
    mMainUi->actionShow_tasks->setEnabled(toggle);
    mMainUi->actionShow_Knock_Spectrum->setEnabled(toggle);
}

void MainWindow::uiDisable()
{
    const bool toggle = false;

    mMainUi->actionConnect->setEnabled(!toggle);
    mMainUi->actionDisconnect->setEnabled(toggle);
    mMainUi->actionGet_Configuration->setEnabled(toggle);
    mMainUi->actionSend_Configuration->setEnabled(toggle);
    mMainUi->actionShow_tasks->setEnabled(toggle);
    mMainUi->actionShow_Knock_Spectrum->setEnabled(toggle);
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
    const int index = mMainUi->tabMain->indexOf(mMainUi->tabFuel);

    if (index >= 0)
        mMainUi->tabMain->setCurrentIndex(index);
}

void MainWindow::showAFRTab()
{
    const int index = mMainUi->tabMain->indexOf(mMainUi->tabAfrMap);

    if (index >= 0)
        mMainUi->tabMain->setCurrentIndex(index);
}

void MainWindow::showStagingTab()
{
    const int index = mMainUi->tabMain->indexOf(mMainUi->tabStaging);

    if (index >= 0)
        mMainUi->tabMain->setCurrentIndex(index);
}

void MainWindow::showAFRTgtTab()
{
    const int index = mMainUi->tabMain->indexOf(mMainUi->tabAfrTarget);

    if (index >= 0)
        mMainUi->tabMain->setCurrentIndex(index);
}

void MainWindow::showIgnTab()
{
    const int index = mMainUi->tabMain->indexOf(mMainUi->tabIgnMap);

    if (index >= 0)
        mMainUi->tabMain->setCurrentIndex(index);
}

void MainWindow::showKnockTab()
{
    const int index = mMainUi->tabMain->indexOf(mMainUi->tabKnock);

    if (index >= 0)
        mMainUi->tabMain->setCurrentIndex(index);
}

void MainWindow::showSettingsTab()
{
    const int index = mMainUi->tabMain->indexOf(mMainUi->tabSettings);

    if (index >= 0)
        mMainUi->tabMain->setCurrentIndex(index);
}

void MainWindow::showTasks()
{
    mTasksWidget->show();
    mTasksWidget->raise();
}

void MainWindow::showKnockGraph()
{
    mKnockGraphWidget->show();
    mKnockGraphWidget->raise();
}

void MainWindow::exportToMTLFile()
{
    mFile.addTable(&mFuelModel);
    mFile.addTable(&mStagingModel);
    mFile.addTable(&mAFRModel);
    mFile.addTable(&mAFRTgtModel);
    mFile.addTable(&mIgnModel);
    mFile.addTable(&mKnockModel);

    mFile.addProperty("Idle", mMainUi->sbIdle->value());
    mFile.addProperty("PitLimiter", mMainUi->sbPitLimiter->value());
    mFile.addProperty("ShiftLight", mMainUi->sbShiftLight->value());
    mFile.addProperty("RpmDiv", mMainUi->dsbRpmDiv->value());
    mFile.addProperty("SpeedDiv", mMainUi->dsbSpeedDiv->value());

    mFile.addProperty("AFRInput", mMainUi->cbAFRInput->currentIndex());

    mFile.addProperty("TPS0",
                      mMainUi->tableSensorTPS->item(0, 0)->data(Qt::EditRole));
    mFile.addProperty("TPS100",
                      mMainUi->tableSensorTPS->item(1, 0)->data(Qt::EditRole));

    mFile.addProperty("AFR8",
                      mMainUi->tableSensorAFR->item(0, 0)->data(Qt::EditRole));
    mFile.addProperty("AFR22",
                      mMainUi->tableSensorAFR->item(1, 0)->data(Qt::EditRole));

}

void MainWindow::importFromMTLFile()
{
    QVariant prop;

    mFile.getProperty("Idle", &prop);
    mMainUi->sbIdle->setValue(prop.toInt());

    mFile.getProperty("PitLimiter", &prop);
    mMainUi->sbPitLimiter->setValue(prop.toInt());

    mFile.getProperty("ShiftLight", &prop);
    mMainUi->sbShiftLight->setValue(prop.toInt());

    mFile.getProperty("RpmDiv", &prop);
    mMainUi->dsbRpmDiv->setValue(prop.toDouble());

    mFile.getProperty("SpeedDiv", &prop);
    mMainUi->dsbSpeedDiv->setValue(prop.toDouble());

    mFile.getProperty("AFRInput", &prop);
    mMainUi->cbAFRInput->setCurrentIndex(prop.toInt());

    mFile.getProperty("TPS0", &prop);
    mMainUi->tableSensorTPS->item(0, 0)->setData(Qt::EditRole, prop);
    mFile.getProperty("TPS100", &prop);
    mMainUi->tableSensorTPS->item(1, 0)->setData(Qt::EditRole, prop);

    mFile.getProperty("AFR8", &prop);
    mMainUi->tableSensorAFR->item(0, 0)->setData(Qt::EditRole, prop);
    mFile.getProperty("AFR22", &prop);
    mMainUi->tableSensorAFR->item(1, 0)->setData(Qt::EditRole, prop);
}

void MainWindow::doFastPolling()
{
    if (mMtl->isConnected())
    {
        emit signalRequestSensors(&mSensorsData);
        if (mKnockGraphWidget->isVisible())
            emit signalRequestKnock(&mKnockSpectrumData);
    }
    else {
        mFastPollingTimer.stop();
    }
}

void MainWindow::doSlowPolling()
{
    if (mMtl->isConnected())
    {
        if (mTasksWidget->isVisible())
            emit signalRequestMonitoring(&mMonitoringData);
    }
    else {
        mSlowPollingTimer.stop();
    }
}

void MainWindow::onSensorsDataReceived(QByteArray *data)
{
    const sensors_t * sensors =  (sensors_t *)data->constData();

    float vAn7 = sensors->an7/1000.0; /* VBAT */
    float vAn8 = sensors->an8/1000.0; /* TPS */
    float vAn9 = sensors->an9/1000.0; /* AFR */
    float tps = sensors->tps/100.0;
    quint16 rpm = sensors->rpm;

    this->setTablesCursor(tps, rpm);
    mMainUi->lVbat->setText(QString::number(vAn7)+tr(" Volts"));

    mMainUi->lTpsVolts->setText(QString::number(vAn8)+tr(" Volts"));
    mMainUi->lTpsPct->setText(QString::number(tps)+tr("%"));

    mMainUi->lAfrVolts->setText(QString::number(vAn9)+tr(" Volts"));
    mMainUi->lRpm->setText(QString::number(rpm)+tr(" Rpm"));
    mMainUi->lRpmHertz->setText(QString::number(sensors->freq1)+tr(" Hertz"));
    mMainUi->lSpeedHertz->setText(QString::number(sensors->freq2)+tr(" Hertz"));
}

void MainWindow::onMonitoringDataReceived(QByteArray *data)
{
    const monitor_t * monitor =  (monitor_t *)data->constData();
    const quint16 maskUsage = 0x3FFF; /* Remove thread state in last 2 bits */
    const quint16 maskState = 0x8000; /* Thread state */
    QTableWidgetItem *item;
    mTasksUi->tableWidget->selectRow(7);

    item = mTasksUi->tableWidget->item(0, 0);
    item->setData(Qt::DisplayRole, float(monitor->ser2 & maskUsage)/100.0);
    if (monitor->ser2 & maskState) mTasksUi->tableWidget->selectRow(0);

    item = mTasksUi->tableWidget->item(1, 0);
    item->setData(Qt::DisplayRole, float(monitor->bdu & maskUsage)/100.0);
    if (monitor->bdu & maskState) mTasksUi->tableWidget->selectRow(0);

    item = mTasksUi->tableWidget->item(2, 0);
    item->setData(Qt::DisplayRole, float(monitor->sdu & maskUsage)/100.0);
    if (monitor->sdu & maskState) mTasksUi->tableWidget->selectRow(1);

    item = mTasksUi->tableWidget->item(3, 0);
    item->setData(Qt::DisplayRole, float(monitor->can & maskUsage)/100.0);
    if (monitor->can & maskState) mTasksUi->tableWidget->selectRow(2);

    item = mTasksUi->tableWidget->item(4, 0);
    item->setData(Qt::DisplayRole, float(monitor->knock & maskUsage)/100.0);
    if (monitor->knock & maskState) mTasksUi->tableWidget->selectRow(3);

    item = mTasksUi->tableWidget->item(5, 0);
    item->setData(Qt::DisplayRole, float(monitor->sensors & maskUsage)/100.0);
    if (monitor->sensors & maskState) mTasksUi->tableWidget->selectRow(4);

    item = mTasksUi->tableWidget->item(6, 0);
    item->setData(Qt::DisplayRole, float(monitor->monitor & maskUsage)/100.0);
    if (monitor->monitor & maskState) mTasksUi->tableWidget->selectRow(5);

    item = mTasksUi->tableWidget->item(7, 0);
    item->setData(Qt::DisplayRole, float(monitor->irq & maskUsage)/100.0);

    item = mTasksUi->tableWidget->item(8, 0);
    item->setData(Qt::DisplayRole, float(monitor->idle & maskUsage)/100.0);
    if (monitor->idle & maskState) mTasksUi->tableWidget->selectRow(7);

}

void MainWindow::OnKnockSpectrumDataReceived(QByteArray *data)
{
    QVector<double> x(SPECTRUM_SIZE), y(SPECTRUM_SIZE);
    QCustomPlot * plot = mKnockGraphUi->mainPlot;
    double max_val = 0;
    double max_freq = 0;

    for (uint i=4; i<SPECTRUM_SIZE; i++)
    {
        x[i] = ((FFT_FREQ*i)/FFT_SIZE);
        y[i] = (KNOCK_MAX/256.0)*(uchar)data->at(i);
        if (y[i] > max_val) {
            max_val = y[i];
            max_freq = x[i];
        }
    }

    mKnockFreqLabel->setText(QString::number(max_val, 'f', 2)+"V at "+
                             QString::number(max_freq, 'f', 2)+"Hz");

    plot->graph(0)->setData(x, y);
    //plot->yAxis->setRange(0, max_val*1.2);
    plot->replot();
}

void MainWindow::onSetTps0Pct(void)
{
    float tps = (float)mMtl->getSensors()->an8/1000.0;
    mMainUi->tableSensorTPS->item(0, 0)->setData(Qt::EditRole, QString::number(tps, 'f', 3));
}

void MainWindow::onSetTps100Pct(void)
{
    float tps = (float)mMtl->getSensors()->an8/1000.0;
    mMainUi->tableSensorTPS->item(1, 0)->setData(Qt::EditRole, QString::number(tps, 'f', 3));
}

void MainWindow::onDataChanged()
{    
    if (mFile.isLoading())
        return;

    mHasChanged = true;
    if (mMainUi->actionAutosave->isChecked())
    {
        this->saveFile();
    }

    if (mMainUi->actionAuto_Send->isChecked())
    {

    }
}

void MainWindow::onHeaderDataNeedSync(int section, Qt::Orientation orientation, const QVariant value)
{
    const int role = Qt::UserRole;
    mFuelModel.setHeaderData(section, orientation, value, role);
    mStagingModel.setHeaderData(section, orientation, value, role);
    mAFRModel.setHeaderData(section, orientation, value, role);
    mAFRTgtModel.setHeaderData(section, orientation, value, role);
    mIgnModel.setHeaderData(section, orientation, value, role);
    mKnockModel.setHeaderData(section, orientation, value, role);
}

void MainWindow::onSimpleError(QString error)
{
    QMessageBox::critical(this, "Error", error);
}

void MainWindow::showNewVersionPopup(QString version)
{
    QMessageBox::information(this,
         tr("New version available"),
         tr("There is a new version available for download: ")+version
         +tr("<br/>You are currently using: ")+__MTL_VER__
         +"<br/><br/><a href='https://github.com/fpoussin/MotoLink/releases/latest'>"
                             +tr("Download here")+"</a>");
}

void MainWindow::showDefaultContextMenu(const QPoint &pos)
{
    QEnhancedTableView* view = (QEnhancedTableView*)this->sender();
    QAction* actions[3];

    QPoint globalPos = view->viewport()->mapToGlobal(pos);
    /* TODO */

    QMenu myMenu;
    actions[0] = myMenu.addAction(tr("Increase selection"));
    actions[1] = myMenu.addAction(tr("Decrease selection"));
    actions[2] = myMenu.addAction(tr("Change selection..."));

    actions[0]->setIcon(QIcon("://oxygen/32x32/actions/list-add.png"));
    actions[1]->setIcon(QIcon("://oxygen/32x32/actions/list-remove.png"));
    actions[2]->setIcon(QIcon("://oxygen/32x32/actions/quickopen-function.png"));

    QAction* selectedItem = myMenu.exec(globalPos);

    if (selectedItem == actions[0])
    {
        // Increase
    }
    else if (selectedItem == actions[1])
    {
        // Decrease
    }
    else if (selectedItem == actions[2])
    {
        // Change
    }
}

void MainWindow::setTablesCursor(uint tps, uint rpm)
{
    int row, col;
    for (int i=0; i<mTablesModelList.size(); i++)
    {
        TableModel* tbl = mTablesModelList.at(i);
        if (tbl->getCell(tps, rpm, &row, &col))
        {
            tbl->highlightCell(row, col);
        }
    }
}
