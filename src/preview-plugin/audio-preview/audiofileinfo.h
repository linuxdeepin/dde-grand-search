/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
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
#ifndef AUDIOINFO_H
#define AUDIOINFO_H

#include <QMap>

namespace GrandSearch {
namespace audio_preview {

class AudioFileInfo
{
public:
    struct AudioMetaData {
        QString title;
        QString artist;
        QString album;
        QString codec;
        QString duration;
    };

    AudioFileInfo();
    AudioMetaData openAudioFile(const QString &file);

private:
    void characterEncodingTransform(AudioMetaData &meta, void *obj);
    QList<QByteArray> detectEncodings(const QByteArray &rawData);
    bool isChinese(const QChar &c);

private:
    QMap<QString, QByteArray> m_localeCodeMap;  // 区域与编码
};

}}

#endif // AUDIOINFO_H
