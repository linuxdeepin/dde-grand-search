#include "downloader.h"

#include <QEventLoop>
#include <QDir>
#include <QRegularExpression>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

using namespace GrandSearch;

Downloader::Downloader(const QString &directory, QObject *parent)
    : QObject(parent),
      m_manager(new QNetworkAccessManager(this)),
      m_downloadDirectory(directory),
      m_finished(false)
{
    connect(m_manager, &QNetworkAccessManager::finished, this, &Downloader::onDownloadFinished);
}

Downloader::~Downloader()
{
    QMutexLocker locker(&mutex);
    m_finished = true;
    foreach (QNetworkReply *reply, m_activeDownloads) {
        if (m_openFiles.contains(reply)) {
            QFile *file = m_openFiles.take(reply);
            if (file->isOpen()) {
                file->close();
            }
            file->deleteLater();
        }
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
        m_finished = false;
        m_activeDownloads.append(reply);
        urlToFileName.insert(url.toString(), url.fileName());
    }

    if (url.fileName().contains(".gguf"))
        connect(reply, &QNetworkReply::downloadProgress, this, &Downloader::onDownloadProgress);
    connect(reply, &QNetworkReply::readyRead, this, &Downloader::onReadyRead);
}

void Downloader::onDownloadFinished(QNetworkReply *reply)
{
    QMutexLocker locker(&mutex);
    disconnect(reply);
    m_activeDownloads.removeAll(reply);

    if (m_openFiles.contains(reply)) {
        QFile *file = m_openFiles.take(reply);
        if (file->isOpen()) {
            file->close();
        }

        if (reply->url().fileName().endsWith("gguf")) {
            auto newUrl = checkCDN(file);
            if (!newUrl.isEmpty()) {
                QNetworkRequest request(newUrl);
                QNetworkReply *nreply = m_manager->get(request);
                m_activeDownloads.append(nreply);
                urlToFileName.insert(nreply->request().url().toString(), reply->url().fileName());
                connect(nreply, &QNetworkReply::downloadProgress, this, &Downloader::onDownloadProgress);
                connect(nreply, &QNetworkReply::readyRead, this, &Downloader::onReadyRead);
            }
        }

        file->deleteLater();
        m_openFiles.remove(reply);
    }

    if (m_activeDownloads.isEmpty() && !m_finished) {
        m_finished = true;
        waitCondition.wakeAll();
        emit downloadFinished();
    }

    reply->deleteLater();
}

void Downloader::onReadyRead()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply) return;

    QString filename = urlToFileName.value(reply->request().url().toString(), reply->request().url().fileName());
    QString localFilePath = m_downloadDirectory + "/" + filename;
    QFile *file = m_openFiles.value(reply);
    if (!file) {
        file = new QFile(localFilePath);
        if (QFile::exists(localFilePath)) {
            QFile::remove(localFilePath);
        }
        if (!file->open(QIODevice::WriteOnly)) {
            qCWarning(logGrandSearch) << "Failed to open file for writing - Path:" << localFilePath
                                      << "Error:" << file->errorString();
            reply->abort();
            delete file;
            return;
        }
        m_openFiles.insert(reply, file);
    }

    file->write(reply->read(reply->bytesAvailable()));
}

void Downloader::cancelDownloads()
{
    QMutexLocker locker(&mutex);
    m_finished = true;
    foreach (QNetworkReply *reply, m_activeDownloads) {
        if (m_openFiles.contains(reply)) {
            QFile *file = m_openFiles.take(reply);
            if (file->isOpen()) {
                file->close();
            }
            file->deleteLater();
        }
        reply->deleteLater();
    }

    m_activeDownloads.clear();
    m_openFiles.clear();
}

QString Downloader::checkCDN(QFile *file)
{
    QString urlStr;
    const QString prefix = R"(<a href=")";
    if (!file->open(QFile::ReadOnly))
        return urlStr;

    QString data = QString::fromUtf8(file->read(prefix.toUtf8().size() + 5));
    if (data.startsWith(prefix)) {
        file->seek(0);
        data = QString::fromUtf8(file->readAll());
        file->close();

        QRegularExpression regex(R"(https?://[^\s"]+)");
        QRegularExpressionMatch match = regex.match(data);

        if (match.hasMatch()) {
            urlStr = match.captured(0);
            urlStr.replace("&amp;", "&");
            qCInfo(logGrandSearch) << "Redirecting download to CDN - File:" << file->fileName()
                                   << "New URL:" << urlStr;
        } else {
            qCWarning(logGrandSearch) << "No CDN URL found in response data - File:" << file->fileName()
                                      << "Data:" << data;
        }
    }

    return urlStr;
}
