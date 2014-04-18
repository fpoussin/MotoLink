#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QtWidgets>

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
};

#endif // MAINWINDOW_H
