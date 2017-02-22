#include "helpviewer.h"
#include "ui_helpviewer.h"
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QUrl>

HelpViewer::HelpViewer(QWidget *parent)
    : QWidget(parent), mUi(new Ui::HelpViewer) {
  mUi->setupUi(this);
}

HelpViewer::~HelpViewer() { delete mUi; }

void HelpViewer::show() {
  QString path;

  path = QDir::tempPath() + "/mtlhelp";

  if (this->cpDir(":/doc/", path)) {
    QDesktopServices::openUrl(QUrl(path + "/html/index.html"));
  } else {
    qDebug("Failed to copy help files");
  }
  // QWidget::show();
}

bool HelpViewer::cpDir(const QString &srcPath, const QString &dstPath) {
  rmDir(dstPath);
  QDir parentDstDir(QFileInfo(dstPath).path());
  if (!parentDstDir.mkdir(QFileInfo(dstPath).fileName()))
    return false;

  QDir srcDir(srcPath);
  foreach (
      const QFileInfo &info,
      srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
    QString srcItemPath = srcPath + "/" + info.fileName();
    QString dstItemPath = dstPath + "/" + info.fileName();
    if (info.isDir()) {
      if (!cpDir(srcItemPath, dstItemPath)) {
        return false;
      }
    } else if (info.isFile()) {
      if (!QFile::copy(srcItemPath, dstItemPath)) {
        return false;
      }
    } else {
      qDebug() << "Unhandled item" << info.filePath() << "in cpDir";
    }
  }
  return true;
}

bool HelpViewer::rmDir(const QString &dirPath) {
  QDir dir(dirPath);
  if (!dir.exists())
    return true;
  foreach (const QFileInfo &info,
           dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot)) {
    if (info.isDir()) {
      if (!rmDir(info.filePath()))
        return false;
    } else {
      if (!dir.remove(info.fileName()))
        return false;
    }
  }
  QDir parentDir(QFileInfo(dirPath).path());
  return parentDir.rmdir(QFileInfo(dirPath).fileName());
}
