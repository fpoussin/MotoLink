#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QCloseEvent>
#include <QString>

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
    
private:
    Ui::MainWindow *ui;
    void setupDefaults(void);
    void setupConnections(void);
    void setupTabShortcuts(void);
    void makeDefaultModel(void);

    QStandardItemModel mDefaultModel;

    QString mCurrentFile;
    quint8 mNumCol;
    quint8 mNumRow;
    bool mHasChanged;
};

#endif // MAINWINDOW_H
