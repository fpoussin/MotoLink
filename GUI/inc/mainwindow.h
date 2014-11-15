#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QItemDelegate>
#include <QCloseEvent>
#include <QString>
#include <QUsb>
#include <QUndoStack>
#include <QTranslator>
#include <QSettings>
#include <QTextBrowser>
#include <QAction>
#include <QUndoView>
#include <QTimer>
#include <QTableView>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QVector>

#include "hrc.h"
#include "updatewizard.h"
#include "helpviewer.h"
#include "motolink.h"
#include "bootloader.h"
#include "commands.h"
#include "tablemodel.h"
#include "qcustomplot.h"
#include "mtlfile.h"
#include "update.h"
#include "qenhancedtableview.h"

#define MAX_RECENT_FILES 5
#define SETTINGS_RECENT_FILES "main/recent_files"
#define SETTINGS_LANGUAGE "main/language"

namespace Ui {
    class MainWindow;
    class Tasks;
    class KnockGraph;
    class HeaderEdit;
}

typedef struct {
    float vAn7; /* VBAT */
    float vAn8; /* TPS */
    float vAn9; /* AFR */
    float tps;
    quint16 afr;
    quint16 knock_value;
    quint16 knock_freq;
    quint16 rpm;
    quint16 freq1;
    quint16 freq2;
} sensors_data_t ;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void Quit(void);
    void closeEvent(QCloseEvent *event);
    void openFile(void);
    void openFile(const QString &filename);
    void saveFile(void);
    void saveFileAs(void);
    void connectMtl(void);
    void disconnectMtl(void);
    void showAbout(void);
    void showUpdateDialog(void);
    void importHrc(void);
    void exportHrc(void);

signals:
    void signalStartupComplete(void);
    void signalRequestSensors(QByteArray *data);
    void signalRequestMonitoring(QByteArray *data);
    void signalRequestKnock(QByteArray *data);

private slots:
    void setLanguageEnglish(void);
    void setLanguageFrench(void);
    void showHelp(void);
    void uiEnable(void);
    void uiDisable(void);
    void updateRecentFilesActions(void);
    void openRecenFile(void);

    void showFuelTab(void);
    void showAFRTab(void);
    void showStagingTab(void);
    void showAFRTgtTab(void);
    void showIgnTab(void);
    void showKnockTab(void);
    void showSettingsTab(void);
    void showTasks(void);
    void showKnockGraph(void);

    void doFastPolling(void);
    void doSlowPolling(void);
    void doSensorsRedraw(void);
    void onSensorsDataReceived(QByteArray *data);
    void onMonitoringDataReceived(QByteArray *data);
    void OnKnockSpectrumDataReceived(QByteArray *data);

    void onSetTps0Pct(void);
    void onSetTps100Pct(void);

    void onDataChanged(void);
    void onHeaderDataNeedSync(int section, Qt::Orientation orientation, const QVariant value);

    void onSimpleError(QString error);

    void showNewVersionPopup(QString version);

    void setTablesCursor(uint tps, uint rpm);

private:
    void setupDefaults(void);
    void setupConnections(void);
    void setupTabShortcuts(void);
    void setupSettings(void);
    void setupKnockGraph(void);
    void makeDefaultModel(void);
    void retranslate(void);

    void exportToMTLFile(void);
    void importFromMTLFile(void);

    Ui::MainWindow *mMainUi;
    Ui::Tasks *mTasksUi;
    Ui::KnockGraph *mKnockGraphUi;
    QWidget *mTasksWidget;
    QWidget *mKnockGraphWidget;
    QCPItemText *mKnockFreqLabel;
    QTranslator mTranslator;
    QSettings mSettings;
    QString mCurrentFile;
    bool mHasChanged;
    Hrc *mHrc;
    Motolink *mMtl;
    UpdateWizard *mUpdateWizard;
    HelpViewer mHelpViewer;
    QUndoStack mUndoStack;
    QUndoView mUndoView;
    QStringList mRecentFiles;
    QAction *mRecentFilesActions[MAX_RECENT_FILES];
    TableModel mFuelModel;
    TableModel mStagingModel;
    TableModel mAFRModel;
    TableModel mAFRTgtModel;
    TableModel mIgnModel;
    TableModel mKnockModel;
    QVector<TableModel*> mTablesModelList;
    QVector<QEnhancedTableView*> mTablesViewList;
    QVector<QSpinBox*> mSpinBoxList;
    Update mUpdate;
    NumberFormatDelegate mDegreeSuffix;
    NumberFormatDelegate mPercentSuffix;
    NumberFormatDelegate mEmptySuffix;
    AfrFormatDelegate mAfrDisplay;

    QTimer mFastPollingTimer;
    QTimer mSlowPollingTimer;
    QTimer mRedrawTimer;
    QByteArray mSensorsData;
    QByteArray mMonitoringData;
    QByteArray mKnockSpectrumData;
    sensors_data_t mSensorsStruct;

    MTLFile mFile;
};

#endif // MAINWINDOW_H
