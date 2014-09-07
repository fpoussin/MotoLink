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

    mFuelModel.setName(tr("Fuel"));
    mStagingModel.setName(tr("Staging"));
    mAFRModel.setName(tr("AFR"));
    mAFRTgtModel.setName(tr("AFR Target"));
    mIgnModel.setName(tr("Ignition"));
    mKnockModel.setName(tr("Knock"));

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
}

void MainWindow::saveFile(void)
{
    if (mCurrentFile.length() == 0)
    {
        this->saveFileAs();
    }

    QFile file(mCurrentFile);

    if (!file.open(QFile::ReadWrite))
    {
        mMainUi->statusBar->showMessage(
                    tr("Failed to open file for writing!"));
        return;
    }

    QXmlStreamWriter stream(&file);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement("Motolink");
    stream.writeStartElement("Info");
    stream.writeAttribute("date", QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    stream.writeAttribute("version", __MTL_VER__);
    stream.writeEndElement(); // Info
    stream.writeStartElement("Settings");
    stream.writeEndElement(); // Settings
    stream.writeStartElement("Tables");
    stream.writeEndElement(); // Tables
    stream.writeEndElement(); // Motolink
    stream.writeEndDocument();

    file.close();

    mMainUi->statusBar->showMessage(
                tr("File saved."));
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

    QObject::connect(&mFuelModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(showFuelTab()));
    QObject::connect(&mStagingModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(showStagingTab()));
    QObject::connect(&mAFRModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(showAFRTab()));
    QObject::connect(&mAFRTgtModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(showAFRTgtTab()));
    QObject::connect(&mIgnModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(showIgnTab()));
    QObject::connect(&mKnockModel, SIGNAL(itemChanged(QStandardItem*)), this, SLOT(showKnockTab()));

    QObject::connect(mMainUi->sbIdle, SIGNAL(valueChanged(int)), this, SLOT(showSettingsTab()));
    QObject::connect(mMainUi->sbPitLimiter, SIGNAL(valueChanged(int)), this, SLOT(showSettingsTab()));
    QObject::connect(mMainUi->sbShiftLight, SIGNAL(valueChanged(int)), this, SLOT(showSettingsTab()));
    QObject::connect(mMainUi->sbThresholdMax, SIGNAL(valueChanged(int)), this, SLOT(showSettingsTab()));
    QObject::connect(mMainUi->sbThresholdMin, SIGNAL(valueChanged(int)), this, SLOT(showSettingsTab()));

    QObject::connect(this, SIGNAL(signalStartupComplete()), &mUpdate, SLOT(getLatestVersion()));
    QObject::connect(&mUpdate, SIGNAL(newVersionAvailable(QString)), mMainUi->statusBar, SLOT(showMessage(QString)));
    QObject::connect(mUpdateWizard, SIGNAL(sendDisconnect()), this, SLOT(disconnectMtl()));


    mMainUi->tableAfrMap->setContextMenuPolicy(Qt::CustomContextMenu);
    mMainUi->tableAfrTgt->setContextMenuPolicy(Qt::CustomContextMenu);
    mMainUi->tableFuel->setContextMenuPolicy(Qt::CustomContextMenu);
    mMainUi->tableIgnMap->setContextMenuPolicy(Qt::CustomContextMenu);
    mMainUi->tableKnk->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(mMainUi->tableAfrMap, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showAfrMapContextMenu(QPoint)));
    QObject::connect(mMainUi->tableAfrTgt, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showAfrTgtContextMenu(QPoint)));
    QObject::connect(mMainUi->tableFuel, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showFuelContextMenu(QPoint)));
    QObject::connect(mMainUi->tableIgnMap, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showIgnContextMenu(QPoint)));
    QObject::connect(mMainUi->tableKnk, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showKnkContextMenu(QPoint)));

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

    QObject::connect(this, SIGNAL(signalUpdateSensors(QByteArray*)), mMtl, SLOT(getSensors(QByteArray*)));
    QObject::connect(mMtl, SIGNAL(sendSensors(QByteArray*)), this, SLOT(receiveSensors(QByteArray*)));

    QObject::connect(this, SIGNAL(signalUpdateMonitoring(QByteArray*)), mMtl, SLOT(getMonitoring(QByteArray*)));
    QObject::connect(mMtl, SIGNAL(sendMonitoring(QByteArray*)), this, SLOT(receiveMonitoring(QByteArray*)));

    QObject::connect(this, SIGNAL(signalUpdateKnock(QByteArray*)), mMtl, SLOT(getKnockSpectrum(QByteArray*)));
    QObject::connect(mMtl, SIGNAL(sendKockSpectrum(QByteArray*)), this, SLOT(receiveKnockSpectrum(QByteArray*)));
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
    plot->xAxis->setRange(0, FFT_FREQ/4);
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
    mMainUi->tabMain->setEnabled(toggle);

    mMainUi->actionSave->setEnabled(toggle);
    mMainUi->actionSave_As->setEnabled(toggle);
    mMainUi->actionImport->setEnabled(toggle);
    mMainUi->actionExport->setEnabled(toggle);
    mMainUi->actionConnect->setEnabled(!toggle);
    mMainUi->actionDisconnect->setEnabled(toggle);
    mMainUi->actionGet_Configuration->setEnabled(toggle);
    mMainUi->actionSend_Configuration->setEnabled(toggle);
    mMainUi->actionAuto_Send->setEnabled(toggle);
    mMainUi->actionShow_tasks->setEnabled(toggle);
    mMainUi->actionShow_Knock_Spectrum->setEnabled(toggle);
}

void MainWindow::uiDisable()
{
    const bool toggle = false;
    mMainUi->tabMain->setEnabled(toggle);

    mMainUi->actionSave->setEnabled(toggle);
    mMainUi->actionSave_As->setEnabled(toggle);
    mMainUi->actionImport->setEnabled(toggle);
    mMainUi->actionExport->setEnabled(toggle);
    mMainUi->actionConnect->setEnabled(!toggle);
    mMainUi->actionDisconnect->setEnabled(toggle);
    mMainUi->actionGet_Configuration->setEnabled(toggle);
    mMainUi->actionSend_Configuration->setEnabled(toggle);
    mMainUi->actionAuto_Send->setEnabled(toggle);
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

void MainWindow::showAfrMapContextMenu(const QPoint &pos)
{
    this->showDefaultContextMenu(pos, mMainUi->tableAfrMap);
}

void MainWindow::showAfrTgtContextMenu(const QPoint &pos)
{
    this->showDefaultContextMenu(pos, mMainUi->tableAfrTgt);
}

void MainWindow::showFuelContextMenu(const QPoint &pos)
{
    this->showDefaultContextMenu(pos, mMainUi->tableFuel);
}

void MainWindow::showIgnContextMenu(const QPoint &pos)
{
    this->showDefaultContextMenu(pos, mMainUi->tableIgnMap);
}

void MainWindow::showKnkContextMenu(const QPoint &pos)
{
    this->showDefaultContextMenu(pos, mMainUi->tableKnk);
}

void MainWindow::showDefaultContextMenu(const QPoint &pos, QTableView *view)
{
    QPoint globalPos = view->viewport()->mapToGlobal(pos);
    /* TODO */

    QMenu myMenu;
    myMenu.addAction(tr("Increase selection"));
    myMenu.addAction(tr("Decrease selection"));
    myMenu.addSeparator();
    myMenu.addAction(tr("Change all cells..."));

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

void MainWindow::doFastPolling()
{
    if (mMtl->isConnected())
    {
        emit signalUpdateSensors(&mSensorsData);
        if (mKnockGraphWidget->isVisible())
            emit signalUpdateKnock(&mKnockSpectrumData);
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
            emit signalUpdateMonitoring(&mMonitoringData);
    }
    else {
        mSlowPollingTimer.stop();
    }
}

void MainWindow::receiveSensors(QByteArray *data)
{
    const sensors_t * sensors =  (sensors_t *)data->constData();

    float vAn7 = sensors->an7/1000.0; /* VBAT */
    float vAn8 = sensors->an8/1000.0; /* TPS */
    float vAn9 = sensors->an9/1000.0; /* AFR */
    mMainUi->lVbat->setText(QString::number(vAn7)+tr(" Volts"));
    mMainUi->lTpsVolts->setText(QString::number(vAn8)+tr(" Volts"));
    mMainUi->lAfrVolts->setText(QString::number(vAn9)+tr(" Volts"));
    mMainUi->lRpmHertz->setText(QString::number(sensors->freq1)+tr(" Hertz"));
    mMainUi->lSpeedHertz->setText(QString::number(sensors->freq2)+tr(" Hertz"));
}

void MainWindow::receiveMonitoring(QByteArray *data)
{
    const monitor_t * monitor =  (monitor_t *)data->constData();
    const quint16 maskUsage = 0x3FFF; /* Remove thread state in last 2 bits */
    const quint16 maskState = 0x8000; /* Thread state */
    QTableWidgetItem *item;
    mTasksUi->tableWidget->selectRow(7);

    item = mTasksUi->tableWidget->item(0, 0);
    item->setData(Qt::DisplayRole, float(monitor->bdu & maskUsage)/100.0);
    if (monitor->bdu & maskState) mTasksUi->tableWidget->selectRow(0);

    item = mTasksUi->tableWidget->item(1, 0);
    item->setData(Qt::DisplayRole, float(monitor->sdu & maskUsage)/100.0);
    if (monitor->sdu & maskState) mTasksUi->tableWidget->selectRow(1);

    item = mTasksUi->tableWidget->item(2, 0);
    item->setData(Qt::DisplayRole, float(monitor->can & maskUsage)/100.0);
    if (monitor->can & maskState) mTasksUi->tableWidget->selectRow(2);

    item = mTasksUi->tableWidget->item(3, 0);
    item->setData(Qt::DisplayRole, float(monitor->knock & maskUsage)/100.0);
    if (monitor->knock & maskState) mTasksUi->tableWidget->selectRow(3);

    item = mTasksUi->tableWidget->item(4, 0);
    item->setData(Qt::DisplayRole, float(monitor->sensors & maskUsage)/100.0);
    if (monitor->sensors & maskState) mTasksUi->tableWidget->selectRow(4);

    item = mTasksUi->tableWidget->item(5, 0);
    item->setData(Qt::DisplayRole, float(monitor->monitor & maskUsage)/100.0);
    if (monitor->monitor & maskState) mTasksUi->tableWidget->selectRow(5);

    item = mTasksUi->tableWidget->item(6, 0);
    item->setData(Qt::DisplayRole, float(monitor->irq & maskUsage)/100.0);

    item = mTasksUi->tableWidget->item(7, 0);
    item->setData(Qt::DisplayRole, float(monitor->idle & maskUsage)/100.0);
    if (monitor->idle & maskState) mTasksUi->tableWidget->selectRow(7);

}

void MainWindow::receiveKnockSpectrum(QByteArray *data)
{
    QVector<double> x(SPECTRUM_SIZE), y(SPECTRUM_SIZE);
    QCustomPlot * plot = mKnockGraphUi->mainPlot;
    double max_val = 0;
    double max_freq = 0;

    for (uint i=0; i<SPECTRUM_SIZE; i++)
    {
        x[i] = (FFT_FREQ*i)/(SPECTRUM_SIZE*4);
        y[i] = (KNOCK_MAX/128.0)*data->at(i)*KNOCK_RATIO;
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

void MainWindow::onSetTps0Pct()
{
    float tps = (float)mMtl->getSensors()->an8/1000.0;
    mMainUi->tableSensorTPS->item(0, 0)->setData(Qt::EditRole, QString::number(tps, 'f', 3));
}

void MainWindow::onSetTps100Pct()
{
    float tps = (float)mMtl->getSensors()->an8/1000.0;
    mMainUi->tableSensorTPS->item(1, 0)->setData(Qt::EditRole, QString::number(tps, 'f', 3));
}
