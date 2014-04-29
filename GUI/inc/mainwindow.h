#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QCloseEvent>
#include <QString>
#include <QUsb>

#include "hrc.h"
#include "updatewizard.h"
#include "motolink.h"
#include "bootloader.h"

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
    void saveFile(void);
    void saveFileAs(void);
    void connectToEcu(void);
    void disconnectFromEcu(void);
    void showAbout(void);
    void showUpdateDialog(void);
    void importHrc(void);
    void exportHrc(void);
    
private:
    void setupDefaults(void);
    void setupConnections(void);
    void setupTabShortcuts(void);
    void makeDefaultModel(void);

    Ui::MainWindow *mUi;
    QStandardItemModel mDefaultModel;
    QString mCurrentFile;
    quint8 mNumCol;
    quint8 mNumRow;
    bool mHasChanged;
    Hrc mHrc;
    QUsb mUsb;
    Motolink mMtl;
    Bootloader mBtl;
    UpdateWizard mUpdateWizard;
};

#endif // MAINWINDOW_H
