#include "hrc.h"

Hrc::Hrc(QObject *parent) :
    QObject(parent)
{
}

bool Hrc::openFile(const QString &filename)
{
    mIntelHexInput.open(filename.toStdString().c_str(), ifstream::in);

    if(!mIntelHexInput.good())
    {
        qWarning() << tr("Error: couldn't open ") << filename;
        return false;
    }

    mIntelHexInput >> mIntelhex;

    return this->checkHexType();
}

bool Hrc::saveFile(const QString &filename)
{
    mIntelHexOutput.open(filename.toStdString().c_str(), ifstream::out);

    if(!mIntelHexOutput.good())
    {
        qWarning() << tr("Error: couldn't open ") << filename;
        return false;
    }

    mIntelHexOutput << mIntelhex;

    return true;
}

bool Hrc::checkHexType(void)
{
    const ulong data_size = mIntelhex.size();

    if (data_size == sizeof(cbr600rr_map_t))
    {
        cbr600rr_map_t tmp;
        mIntelhex.getData((uchar*)&tmp);

        if (strcmp(CBR600RR07_SIGN, tmp.signature) == 0)
        {
            qDebug() << tr("CBR600RR7 Map detected");
            mCbr600rr_map = tmp; /* Implicit copy */
            return true;
        }
    }

    qWarning() << tr("Unknown Map Type");
    return false;
}
