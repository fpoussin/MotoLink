#ifndef MTLFILE_H
#define MTLFILE_H

#include "tablemodel.h"
#include <QFile>
#include <QMap>
#include <QObject>
#include <QString>
#include <QVariant>

class MTLFile : public QObject {
  Q_OBJECT
public:
  explicit MTLFile(QObject *parent = 0);
  bool addTable(TableModel *table);
  bool getTable(const QString &name, TableModel *table);
  bool rmTable(const QString &name);
  bool rmTable(TableModel *const table);

  bool addProperty(const QString &name, QVariant value);
  bool getProperty(const QString &name, QVariant *value);
  bool rmProperty(const QString &name);

  bool isLoading(void);

public slots:
  bool write(QFile *file);
  bool read(QFile *file);

signals:
  void readFailed(QString);

private:
  QMap<QString, TableModel *> mTableList;
  QMap<QString, QVariant> mPropList;
  bool mLoading;
};

#endif // MTLFILE_H
