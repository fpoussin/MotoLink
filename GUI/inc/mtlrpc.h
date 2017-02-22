#ifndef MTLRPC_H
#define MTLRPC_H

#include <QObject>
#include <QJsonDocument>
#include "qjsonrpcmessage.h"

class MTLRpc : public QObject {
  Q_OBJECT
public:
  explicit MTLRpc(QObject *parent = 0);
  ~MTLRpc();

signals:

public slots:
  void request(const QString &request, QString *response);

private:
  QJsonRpcMessage createBasicRequest(const QString &method, const QJsonArray &params);
  QJsonRpcMessage createBasicRequest(const QString &method, const QJsonObject &namedParameters);

  QJsonRpcMessage::Type mType;
  QJsonRpcMessage mRequest;
  QJsonRpcMessage mResponse;
};

#endif // MTLRPC_H
