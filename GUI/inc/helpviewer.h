#ifndef HELPVIEWER_H
#define HELPVIEWER_H

#include <QWidget>

namespace Ui {
class HelpViewer;
}

class HelpViewer : public QWidget
{
    Q_OBJECT

public:
    explicit HelpViewer(QWidget *parent = 0);
    ~HelpViewer();

public slots:
    void show(void);
    bool cpDir(const QString &srcPath, const QString &dstPath);
    bool rmDir(const QString &dirPath);

private:
    Ui::HelpViewer *mUi;
};

#endif // HELPVIEWER_H
