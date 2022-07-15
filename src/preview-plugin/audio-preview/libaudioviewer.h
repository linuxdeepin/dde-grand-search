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
