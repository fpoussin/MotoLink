#ifndef HRC_H
#define HRC_H

#include <QObject>
#include <QDebug>
#include "intelhexclass.h"
#include <fstream>
#include "datastructures.h"

class Hrc : public QObject
{
    Q_OBJECT
public:
    explicit Hrc(QObject *parent = 0);

signals:

public slots:
    bool openFile(const QString &filename);
    bool saveFile(const QString &filename);

private slots:
    bool checkHexType(void);

private:
    std::ifstream mIntelHexInput;
    std::ofstream mIntelHexOutput;
    intelhex mIntelhex;
    QString mIntelHexLine;

    cbr600rr_map_t mCbr600rr_map;

};

#endif // HRC_H
