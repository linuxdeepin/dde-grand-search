#include "downloader.h"
#include <QEventLoop>
#include <QDir>

using namespace GrandSearch;

Downloader::Downloader(const QString &directory, QObject *parent) : QObject(parent),
    m_manager(new QNetworkAccessManager(this)),
    m_downloadDirectory(directory),
    m_finished(false)
{
    connect(m_manager, &QNetworkAccessManager::finished, this, &Downloader::onDownloadFinished);
}

Downloader::~Downloader()
{
    QMutexLocker locker(&mutex);
    foreach (QNetworkReply *reply, m_activeDownloads) {
        reply->deleteLater();
    }
    m_manager->deleteLater();
}

void Downloader::addDownloadTask(const QUrl &url)
{
    QNetworkRequest request(url);
    QNetworkReply *reply = m_manager->get(request);
    {
        QMutexLocker locker(&mutex);
        m_activeDownloads.append(reply);
    }
}

void Downloader::onDownloadFinished(QNetworkReply *reply)
{
    saveFile(reply, reply->request().url().fileName());
    reply->deleteLater();
    {
        QMutexLocker locker(&mutex);
        m_activeDownloads.removeAll(reply);
        if (m_activeDownloads.isEmpty() && !m_finished) {
            m_finished = true;
            waitCondition.wakeAll();
            emit downloadFinished();
        }
    }
}

void Downloader::saveFile(QNetworkReply *reply, const QString &filename)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Download failed:" << reply->errorString();
        return;
    }

    QString localFilePath = m_downloadDirectory + "/" + filename;
    QFile file(localFilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Unable to open" << localFilePath << "for writing:" << file.errorString();
        return;
    }

    file.write(reply->readAll());
    file.close();
}
