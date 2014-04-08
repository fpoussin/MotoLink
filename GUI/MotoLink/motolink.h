#ifndef MOTOLINK_H
#define MOTOLINK_H

#include <QMainWindow>

namespace Ui {
class MotoLink;
}

class MotoLink : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MotoLink(QWidget *parent = 0);
    ~MotoLink();
    
private:
    Ui::MotoLink *ui;
};

#endif // MOTOLINK_H
