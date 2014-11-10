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
    mLoading = false;
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

bool MTLFile::isLoading()
{
    return mLoading;
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
        writer.writeStartElement("Property");
        writer.writeAttribute("Name", pi.key());
        writer.writeCharacters(pi.value().toString());
        writer.writeEndElement();
    }
    writer.writeEndElement(); // Properties

    TableModel* firstTable = mTableList.first();
    int rows = firstTable->rowCount();
    int columns = firstTable->columnCount();

    writer.writeStartElement("Rows");
    writer.writeAttribute("count", QString::number(rows));
    for (int i=0; i<rows; i++)
    {
        writer.writeStartElement("Row");
        writer.writeCharacters(firstTable->headerData(i, Qt::Vertical, Qt::EditRole).toString());
        writer.writeEndElement(); // Row
    }
    writer.writeEndElement(); // Rows
    writer.writeStartElement("Columns");
    writer.writeAttribute("count", QString::number(columns));
    for (int i=0; i<columns; i++)
    {
        writer.writeStartElement("Column");
        writer.writeCharacters(firstTable->headerData(i, Qt::Horizontal, Qt::EditRole).toString());
        writer.writeEndElement(); // Column
    }
    writer.writeEndElement(); // Columns

    writer.writeStartElement("Tables");
    QMapIterator<QString, TableModel*> ti(mTableList);
    while (ti.hasNext()) {
        ti.next();
        writer.writeStartElement("Table");
        writer.writeAttribute("Name", ti.value()->getName());
        uint rows = ti.value()->rowCount();
        uint columns = ti.value()->columnCount();

        for (uint r = 0; r < rows; r++)
        {
            writer.writeStartElement("Row");
            for (uint c = 0; c < columns; c++)
            {
                writer.writeStartElement("Column");
                writer.writeCharacters(ti.value()->index(r, c).data().toString());
                writer.writeEndElement(); // Column
            }
            writer.writeEndElement(); // Row
        }
        writer.writeEndElement(); // Table
    }
    writer.writeEndElement(); // Tables
    writer.writeEndElement(); // Motolink
    writer.writeEndDocument();

    return true;
}

bool MTLFile::read(QFile *file)
{
    QDomDocument domDocument;
    domDocument.setContent(file, true);

    QDomElement root(domDocument.documentElement());

    if (root.tagName() != "Motolink") {

        emit readFailed(tr("Not a Motolink file"));
        return false;
    }

    QDomElement properties(root.firstChildElement("Properties"));
    QDomElement rows(root.firstChildElement("Rows"));
    QDomElement columns(root.firstChildElement("Columns"));
    QDomElement tables(root.firstChildElement("Tables"));

    if (properties.isNull() || tables.isNull()
            || rows.isNull() || columns.isNull()) {

        emit readFailed(tr("Data missing"));
        return false;
    }

    mLoading = true;
    TableModel* firstTable = mTableList.first();
    QDomElement prop(properties.firstChildElement("Property"));
    while (!prop.isNull())
    {
        mPropList.insert(prop.attribute("Name"),
                         prop.text());
        prop = prop.nextSiblingElement("Property");
    }

    QDomElement row(rows.firstChildElement("Row"));
    int rowNum = 0;
    while (!row.isNull())
    {
        QVariant value = row.text();
        firstTable->setHeaderData(rowNum++, Qt::Vertical, value);
        row = row.nextSiblingElement("Row");
    }

    QDomElement column(columns.firstChildElement("Column"));
    int colNum = 0;
    while (!column.isNull())
    {
        QVariant value = column.text();
        firstTable->setHeaderData(colNum++, Qt::Horizontal, value);
        column = column.nextSiblingElement("Column");
    }

    QDomElement table(tables.firstChildElement("Table"));
    while (!table.isNull())
    {
        QString tableName = table.attribute("Name");

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

        QDomElement tps(table.firstChildElement("Row"));
        int r = 0;
        while (!tps.isNull())
        {
            QDomElement rpm(tps.firstChildElement("Column"));

            int c = 0;
            while (!rpm.isNull())
            {
                QString value(rpm.text());

                tableModel->setValue(r, c, value);
                rpm = rpm.nextSiblingElement("Column");
                c++;
            }

            tps = tps.nextSiblingElement("Row");
            r++;
        }
        table = table.nextSiblingElement("Table");
    }

    mLoading = false;
    return true;
}
