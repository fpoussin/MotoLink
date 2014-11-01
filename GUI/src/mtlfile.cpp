#include "mtlfile.h"
#include "tablemodel.h"
#include <QXmlStreamWriter>
#include <QDateTime>
#include <QDomDocument>
#include <QDomElement>
#include <QDebug>

MTLFile::MTLFile(QObject *parent) :
    QObject(parent)
{
}

bool MTLFile::addTable(TableModel* table)
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
    QXmlStreamWriter writer(file);
    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeStartElement("Motolink");
    writer.writeStartElement("Info");
    writer.writeAttribute("date",
                          QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    writer.writeAttribute("version", __MTL_VER__);
    writer.writeEndElement(); // Info
    writer.writeStartElement("Settings");
    writer.writeEndElement(); // Settings

    writer.writeStartElement("Properties");
    QMapIterator<QString, QVariant> pi(mPropList);
    while (pi.hasNext()) {
        pi.next();
        writer.writeStartElement(pi.key());
        writer.writeCharacters(pi.value().toString());
        writer.writeEndElement();
    }
    writer.writeEndElement(); // Properties

    writer.writeStartElement("Tables");
    QMapIterator<QString, TableModel*> ti(mTableList);
    while (ti.hasNext()) {
        ti.next();
        writer.writeStartElement("Table");
        writer.writeAttribute("name", ti.value()->getName());
        uint rows = ti.value()->rowCount();
        uint columns = ti.value()->columnCount();

        for (uint r = 0; r < rows; r++)
        {
            writer.writeStartElement("TPS");
            writer.writeAttribute("v", ti.value()->headerData(r, Qt::Vertical).toString());
            for (uint c = 0; c < columns; c++)
            {
                writer.writeStartElement("RPM");
                writer.writeAttribute("v",
                                      ti.value()->headerData(c, Qt::Horizontal).toString());
                writer.writeCharacters(ti.value()->index(r, c).data().toString());
                writer.writeEndElement();
            }
            writer.writeEndElement();
        }
        writer.writeEndElement();
    }
    writer.writeEndElement(); // Tables
    writer.writeEndElement(); // Motolink
    writer.writeEndDocument();

    return false;
}

bool MTLFile::read(QFile *file)
{
    QDomDocument domDocument;
    domDocument.setContent(file, true);

    QDomElement root(domDocument.documentElement());

    if (root.tagName() != "Motolink") {

        qWarning() << tr("Could not parse XML data");
        emit readFailed();

        return false;
    }

    QDomElement properties(root.firstChildElement("Properties"));
    QDomElement tables(root.firstChildElement("Tables"));

    if (properties.isNull() || tables.isNull()) {

        qWarning() << tr("Could not parse XML data");
        emit readFailed();

        return false;
    }

    QDomElement prop(properties.firstChildElement());
    while (!prop.isNull())
    {
        qDebug() << prop.nodeName() <<  prop.text();
        mPropList.insert(prop.nodeName(),
                         prop.text());
        prop = prop.nextSiblingElement();
    }

    QDomElement table(tables.firstChildElement("Table"));

    //QDomNodeList tablesList = tables.childNodes();
    //for (uint n=0; n<tablesList.length(); n++)
    while (!table.isNull())
    {
        //table = tablesList.at(n).toElement();
        QString tableName = table.attribute("name");

        qDebug() << tr("Table: ") << tableName;

        // Convert XML data to Model
        if (!mTableList.contains(tableName))
        {
            qWarning() << tr("Unknown table: ") << tableName;

            table = table.nextSiblingElement("Table");
            continue;
        }

        TableModel* tableModel = mTableList.value(tableName);
        if (tableModel == NULL)
        {
            qWarning() << tr("Could not allocate: ") << tableName;
            table = table.nextSiblingElement("Table");
            continue;
        }

        QDomElement tps(table.firstChildElement("TPS"));
        while (!tps.isNull())
        {
            QString tpStr(tps.attribute("v").remove("%"));
            QDomElement rpm(tps.firstChildElement("RPM"));

            while (!rpm.isNull())
            {
                QString rpmStr(rpm.attribute("v"));
                QString value(rpm.text());

                tableModel->setValue(tpStr.toInt(),
                                     rpmStr.toInt(),
                                     value);
                rpm = rpm.nextSiblingElement("RPM");
            }

            tps = tps.nextSiblingElement("TPS");
        }
        table = table.nextSiblingElement("Table");
    }

    return true;
}
