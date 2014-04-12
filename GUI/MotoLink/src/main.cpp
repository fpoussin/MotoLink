#include "motolink.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MotoLink w;
    w.show();
    
    return a.exec();
}
