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

#include "hrc.h"
#include "updatewizard.h"
#include "helpviewer.h"
#include "motolink.h"
#include "bootloader.h"
#include "commands.h"
#include "tablemodel.h"

#include "update.h"

#define MAX_RECENT_FILES 5
#define SETTINGS_RECENT_FILES "main/recent_files"
#define SETTINGS_LANGUAGE "main/language"

namespace Ui {
    class MainWindow;
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
    void importHrc(void);
    void exportHrc(void);

signals:
    void startupComplete(void);
    void sendUpdateSensors(QByteArray *data);

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

    void showFuelContextMenu(const QPoint& pos);

    void updateSensors(void);
    void receiveSensors(QByteArray *data);

private:
    void setupDefaults(void);
    void setupConnections(void);
    void setupTabShortcuts(void);
    void setupSettings(void);
    void makeDefaultModel(void);
    void retranslate(void);

    Ui::MainWindow *mUi;
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
    Update mUpdate;

    QTimer mSensorsTimer;
    QByteArray mSensorsData;
};

#endif // MAINWINDOW_H
