#ifndef UPDATE_H
#define UPDATE_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QScriptEngine>
#include <QScriptValueIterator>

class Update : public QObject
{
    Q_OBJECT
public:
    explicit Update(QObject *parent = 0);

signals:
    void newVersionAvailable(QString version);

public slots:
    void getLatestVersion(void);
    void onResult(QNetworkReply* reply);

private:
    QUrl mReleasesUrl;
    QNetworkAccessManager mNetworkManager;
    QString mCurrentVersion;
    QString mNewVersion;
};

#endif // UPDATE_H
