#include "update.h"
#include <QTimer>

Update::Update(QObject *parent) :
    QObject(parent)
{
    mReleasesUrl.setUrl("https://api.github.com/repos/fpoussin/motolink/releases");
    connect(&mNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onResult(QNetworkReply*)));
    mCurrentVersion = __MTL_VER__;
    mNewVersion = mCurrentVersion;
}

void Update::getLatestVersion()
{
    qDebug("Checking version...");
    QNetworkRequest request;
    request.setUrl(mReleasesUrl);
    QNetworkReply *r = mNetworkManager.get(request);
    QTimer t;
    t.singleShot(3000, r, SLOT(abort()));
}

void Update::onResult(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError || reply->size() < 1) {
        qDebug("Unable to get latest version");
        return;
    }

    QJsonDocument doc(QJsonDocument::fromJson(reply->readAll()));
    delete reply;

    if (!doc.isArray()) {
        qDebug("Unable to get latest version");
        return;
    }

    QJsonArray array(doc.array());
    QJsonValue val(array.at(0));
    QJsonObject release(val.toObject());

    if (release.isEmpty()) {
        qDebug("Unable to get latest version");
        return;
    }

    QString version = release["tag_name"].toString();
    version.remove(QRegExp("[a-zA-Z]"));

    if (version.isEmpty()) {
        qDebug("Unable to get latest version");
        return;
    }

    mNewVersion = version;
    if (mCurrentVersion.compare(mNewVersion) < 0)
    {
        qDebug("New version available: %s", version.toStdString().c_str());
        emit newVersionAvailable(version);
    }
    else
    {
        qDebug("Using latest version: %s", version.toStdString().c_str());
    }

}
