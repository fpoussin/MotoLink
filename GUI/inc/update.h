#ifndef UPDATE_H
#define UPDATE_H

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QUrl>

#ifdef WIN32
// Linked libraries
#pragma comment(lib, "libeay32.lib")
#pragma comment(lib, "ssleay32.lib")
#endif

class Update : public QObject {
  Q_OBJECT
public:
  explicit Update(QObject *parent = 0);

signals:
  void newVersionAvailable(QString version);

public slots:
  void getLatestVersion(void);
  void onResult(QNetworkReply *reply);

private:
  QUrl mReleasesUrl;
  QNetworkAccessManager mNetworkManager;
  QString mCurrentVersion;
  QString mNewVersion;
};

#endif // UPDATE_H
