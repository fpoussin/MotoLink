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
    class Logs;
}

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

signals:
    void signalStartupComplete(void);

private slots:
    void setLanguageEnglish(void);
    void setLanguageFrench(void);
    void showHelp(void);
    void uiEnable(void);
    void uiDisable(void);
    void updateRecentFilesActions(void);
    void openRecenFile(void);

    void showAFRTab(void);
    void showAFRTgtTab(void);
    void showKnockTab(void);
    void showSettingsTab(void);
    void showTasks(void);
    void showKnockGraph(void);
    void showSerialData(void);

    void showLogs(void);
    void log(const QString & msg);

    void doFastPolling(void);
    void doSlowPolling(void);
    void doTablesPolling(void);
    void doSensorsRedraw(void);
    void onMonitoringReceived(const TaskList *monitoring);
    void onKnockSpectrumReceived(const QByteArray * data);
    void onTablesReceived(const quint8 * afr, const quint8 * knock);
    void onSerialDataReceived(const QByteArray * data);

    void onSetTps0Pct(void);
    void onSetTps100Pct(void);

    void onDataChanged(void);
    void onHeadersNeedSync(int section, Qt::Orientation orientation, const QVariant value);

    void onSimpleError(QString error);

    void showNewVersionPopup(QString version);

    void setTablesCursorFromSensors(uint tps, uint rpm);
    void setTablesCursor(uint row, uint col);

    void onReadMtlSettings(void);
    void onWriteMtlSettings(void);

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
    Ui::Logs *mLogsUi;
    Ui::Logs *mSerialLogsUi;
    QWidget *mTasksWidget;
    QWidget *mKnockGraphWidget;
    QWidget *mLogsWidget;
    QWidget *mSerialLogsWidget;
    QCPItemText *mKnockFreqLabel;
    QTranslator mTranslator;
    QSettings mSettings;
    QString mCurrentFile;
    bool mHasChanged;
    Motolink *mMtl;
    UpdateWizard *mUpdateWizard;
    HelpViewer mHelpViewer;
    QUndoStack mUndoStack;
    QUndoView mUndoView;
    QStringList mRecentFiles;
    QAction *mRecentFilesActions[MAX_RECENT_FILES];
    TableModel mAFRModel;
    TableModel mAFRTgtModel;
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
    QTimer mTablesTimer;
    QTimer mRedrawTimer;
    QByteArray mSensorsData;
    QByteArray mMonitoringData;
    QByteArray mKnockSpectrumData;

    MTLFile mFile;
};

#endif // MAINWINDOW_H
