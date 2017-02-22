#include "mtlrpc.h"

MTLRpc::MTLRpc(QObject *parent) : QObject(parent) {


}

MTLRpc::~MTLRpc()
{

}

void MTLRpc::request(const QString &request, QString *response) {


}

QJsonRpcMessage MTLRpc::createBasicRequest(const QString &method,
                                           const QJsonArray &params) {

}

QJsonRpcMessage MTLRpc::createBasicRequest(const QString &method,
                                           const QJsonObject &namedParameters) {

}
