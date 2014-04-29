#include "hrc.h"

Hrc::Hrc(QObject *parent) :
    QObject(parent)
{
}

bool Hrc::openFile(const QString &filename)
{
    this->mIntelHexInput.open(filename.toStdString().c_str(), ifstream::in);

    if(!this->mIntelHexInput.good())
    {
        qWarning() << "Error: couldn't open " << filename;
        return false;
    }

    mIntelHexInput >> this->mIntelhex;

    return this->checkHexType();
}

bool Hrc::saveFile(const QString &filename)
{
    this->mIntelHexOutput.open(filename.toStdString().c_str(), ifstream::out);

    if(!this->mIntelHexOutput.good())
    {
        qWarning() << "Error: couldn't open " << filename;
        return false;
    }

    mIntelHexOutput << this->mIntelhex;

    return true;
}

bool Hrc::checkHexType(void)
{
    const ulong data_size = this->mIntelhex.size();

    if (data_size == sizeof(cbr600rr_map_t))
    {
        cbr600rr_map_t tmp;
        this->mIntelhex.getData((uchar*)&tmp);

        if (strcmp(CBR600RR07_SIGN, tmp.signature) == 0)
        {
            qDebug("CBR600RR7 Map detected");
            this->mCbr600rr_map = tmp; /* Implicit copy */
            return true;
        }
    }

    qWarning("Unknown Map Type");
    return false;
}
