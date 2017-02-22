#ifndef MTLRPC_H
#define MTLRPC_H

#include "qjsonrpcsocket.h"
#include <QObject>

class MTLRpc : public QObject {
  Q_OBJECT
public:
  explicit MTLRpc(QObject *parent = 0);

signals:

public slots:

private:
  QJsonRpcSocket *mClient;
};

#endif // MTLRPC_H
