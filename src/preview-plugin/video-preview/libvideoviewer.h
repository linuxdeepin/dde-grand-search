/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
