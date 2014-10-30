#include "mtlfile.h"
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDateTime>

MTLFile::MTLFile(QObject *parent) :
    QObject(parent)
{
}

bool MTLFile::addTable(TableModel* const table)
{
    mTableList.insert(table->getName(), table);

    return false;
}

bool MTLFile::getTable(const QString &name, TableModel *table)
{
    if (!mTableList.contains(name))
        return false;

    table = mTableList.value(name);
    return true;
}

bool MTLFile::rmTable(const QString &name)
{
    return mTableList.remove(name);
}

bool MTLFile::rmTable(TableModel* const table)
{
    QString key;

    key = mTableList.key(table);

    if (key.isEmpty())
        return false;

    mTableList.remove(key);
    return true;
}

bool MTLFile::addProperty(const QString &name, QVariant value)
{
    mPropList.insert(name, value);

    return true;
}

bool MTLFile::getProperty(const QString &name, QVariant *value)
{
    if (!mPropList.contains(name))
        return false;

    *value = mPropList.value(name);
    return true;
}

bool MTLFile::rmProperty(const QString &name)
{
    return mPropList.remove(name);
}

bool MTLFile::write(QFile *file)
{
    QXmlStreamWriter stream(file);
    stream.setAutoFormatting(true);
    stream.writeStartDocument();
    stream.writeStartElement("Motolink");
    stream.writeStartElement("Info");
    stream.writeAttribute("date",
                          QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    stream.writeAttribute("version", __MTL_VER__);
    stream.writeEndElement(); // Info
    stream.writeStartElement("Settings");
    stream.writeEndElement(); // Settings

    stream.writeStartElement("Properties");
    QHashIterator<QString, QVariant> pi(mPropList);
    while (pi.hasNext()) {
        pi.next();
        stream.writeStartElement(pi.key());
        stream.writeCharacters(pi.value().toString());
        stream.writeEndElement();
    }
    stream.writeEndElement(); // Properties

    stream.writeStartElement("Tables");
    QHashIterator<QString, TableModel*> ti(mTableList);
    while (ti.hasNext()) {
        ti.next();
        stream.writeStartElement(ti.value()->getName());
        uint rows = ti.value()->rowCount();
        uint columns = ti.value()->columnCount();

        for (uint r = 0; r < rows; r++)
        {
            stream.writeStartElement(
                        ti.value()->headerData(r, Qt::Vertical).toString());
            for (uint c = 0; c < columns; c++)
            {
                stream.writeStartElement(
                            ti.value()->headerData(c, Qt::Horizontal).toString());
                stream.writeCharacters(ti.value()->index(r, c).data().toString());
                stream.writeEndElement();
            }
            stream.writeEndElement();
        }
        stream.writeEndElement();
    }
    stream.writeEndElement(); // Tables
    stream.writeEndElement(); // Motolink
    stream.writeEndDocument();

    return false;
}

bool MTLFile::read(QFile *file)
{
    QXmlStreamReader stream(file);

    return false;
}
