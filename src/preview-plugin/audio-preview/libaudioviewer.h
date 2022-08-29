// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LIBAUDIOVIEWER_H
#define LIBAUDIOVIEWER_H

#include <QObject>

class QLibrary;
namespace GrandSearch {

typedef void (*GetMovieInfoFunc)(const QUrl &url, QJsonObject *json);

class LibAudioViewer : public QObject
{
    Q_OBJECT
public:
    explicit LibAudioViewer(QObject *parent = nullptr);
    bool initLibrary();
    bool getDuration(const QUrl &url, qint64 &duration);
signals:

private:
    QLibrary *m_imageLib = nullptr;
    GetMovieInfoFunc m_getMovieInfoFunc = nullptr;
    bool loaded = false;
};
}

#endif // LIBAUDIOVIEWER_H
