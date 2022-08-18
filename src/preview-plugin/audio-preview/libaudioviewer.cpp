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
#include "libaudioviewer.h"

#include <QLibrary>
#include <QDebug>
#include <QJsonObject>
#include <QJsonValue>

using namespace GrandSearch;

LibAudioViewer::LibAudioViewer(QObject *parent) : QObject(parent)
{

}

bool LibAudioViewer::initLibrary()
{
    if (m_imageLib)
        return true;

    if (loaded) // load failed.
        return false;
    loaded = true;

    qInfo() << "load libimageviewer.so....";
    m_imageLib = new QLibrary("libimageviewer.so", this);
    if (!m_imageLib->load()) {
        qWarning() << "fail to load libimageviewer" << m_imageLib->errorString();
        delete m_imageLib;
        m_imageLib = nullptr;
        return false;
    }

    m_getMovieInfoFunc = reinterpret_cast<GetMovieInfoFunc>(m_imageLib->resolve("getMovieInfoByJson"));
    if (!m_getMovieInfoFunc) {
        qWarning() << "can not find getMovieInfoByJson.";
    }

    qInfo() << "load successfully";
    return true;
}

bool LibAudioViewer::getDuration(const QUrl &url, qint64 &duration)
{
    if (!m_getMovieInfoFunc)
        return false;

    QJsonObject json;
    m_getMovieInfoFunc(url, &json);
    if (json.contains("Base")) {
       const QJsonObject &base = json.value("Base").toObject();
       const QJsonValue &durationJson = base.value("Duration");
       if (durationJson.isString()) {
           QString timeStr = durationJson.toString();
           qDebug() << "get duration" << timeStr;
           QStringList times = timeStr.split(':');
           if (times.size() == 3) {
               duration = times.at(0).toInt() * 3600 + times.at(1).toInt() * 60 + times.at(2).toInt();
               return true;
           }
       }
    }

    duration = -1;
    qDebug() << "get duration failed";
    return false;
}
