// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LIBVIDEOVIEWER_H
#define LIBVIDEOVIEWER_H

#include <QObject>

class QLibrary;
namespace GrandSearch {

typedef void (*GetMovieCoverFunc)(const QUrl &url, const QString &savePath, QImage *image);
typedef void (*GetMovieInfoFunc)(const QUrl &url, QJsonObject *json);

class LibVideoViewer : public QObject
{
    Q_OBJECT
public:
    explicit LibVideoViewer(QObject *parent = nullptr);
    ~LibVideoViewer() override;
    bool initLibrary();
    bool getMovieInfo(const QUrl &url, QSize &dimension, qint64 &duration);
    bool getMovieCover(const QUrl &url, QImage &image);
signals:

public slots:
private:
    QLibrary *m_imageLib = nullptr;
    GetMovieCoverFunc m_getMovieCoverFunc = nullptr;
    GetMovieInfoFunc m_getMovieInfoFunc = nullptr;
};

}

#endif // LIBVIDEOVIEWER_H
