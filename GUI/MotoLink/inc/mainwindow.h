#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    
private:
    Ui::MainWindow *ui;
    void setupDefaults(void);
    void setupConnections(void);
};

#endif // MAINWINDOW_H
