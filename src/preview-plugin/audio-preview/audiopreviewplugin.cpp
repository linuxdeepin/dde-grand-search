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
#include "audiopreviewplugin.h"
#include "audiofileinfo.h"
#include "audioview.h"
#include "global/commontools.h"

#include <QFileInfo>
#include <QRect>
#include <QDateTime>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

AudioPreviewPlugin::AudioPreviewPlugin(QObject *parent)
    : QObject (parent)
    , GrandSearch::PreviewPlugin()
{

}

AudioPreviewPlugin::~AudioPreviewPlugin()
{
    if (m_audioView)
        m_audioView->deleteLater();
}

void AudioPreviewPlugin::init(QObject *proxyInter)
{
    Q_UNUSED(proxyInter)
    if (!m_audioView)
        m_audioView = new AudioView();
}

bool AudioPreviewPlugin::previewItem(const GrandSearch::ItemInfo &item)
{
    const QString path = item.value(PREVIEW_ITEMINFO_ITEM);
    if (path.isEmpty())
        return false;

    m_audioView->setItemInfo(item);

    AudioFileInfo afi;
    AudioFileInfo::AudioMetaData amd = afi.openAudioFile(path);
    // 歌手
    auto artist = qMakePair<QString, QString>(tr("Artist:"), amd.artist.isEmpty() ? "--" : amd.artist);
    m_detailInfos.push_back(artist);
    // 专辑
    auto album = qMakePair<QString, QString>(tr("Album:"), amd.album.isEmpty() ? "--" : amd.album);
    m_detailInfos.push_back(album);
    // 时长
    if (amd.duration.isEmpty()) {
        AVFormatContext *avFmormat = avformat_alloc_context();
        avformat_open_input(&avFmormat, path.toStdString().c_str(), nullptr, nullptr);
        if (avFmormat) {
            avformat_find_stream_info(avFmormat, nullptr);
            qint64 dura = avFmormat->duration / (qint64)AV_TIME_BASE;
            if (dura >= 0)
                amd.duration = GrandSearch::CommonTools::durationString(dura);
        }
        avformat_close_input(&avFmormat);
        avformat_free_context(avFmormat);
    }
    auto duration = qMakePair<QString, QString>(tr("Duration:"), amd.duration.isEmpty() ? "--" : amd.duration);
    m_detailInfos.push_back(duration);
    // 类型
    QFileInfo fileInfo(path);
    auto suffix = fileInfo.suffix();
    auto fileType = qMakePair<QString, QString>(tr("Type:"), suffix.isEmpty() ? "--" : suffix);
    m_detailInfos.push_back(fileType);
    // 位置
    auto location = qMakePair<QString, QString>(tr("Location:"), fileInfo.absoluteFilePath());
    m_detailInfos.push_back(location);
    // 修改时间
    auto time = fileInfo.lastModified();
    auto timeModified = qMakePair<QString, QString>(tr("Time modified:"), time.toString(GrandSearch::CommonTools::dateTimeFormat()));
    m_detailInfos.push_back(timeModified);

    m_item = item;
    return true;
}

GrandSearch::ItemInfo AudioPreviewPlugin::item() const
{
    return m_item;
}

bool AudioPreviewPlugin::stopPreview()
{
    return true;
}

QWidget *AudioPreviewPlugin::contentWidget() const
{
    return m_audioView;
}

GrandSearch::DetailInfoList AudioPreviewPlugin::getAttributeDetailInfo() const
{
    return m_detailInfos;
}

QWidget *AudioPreviewPlugin::toolBarWidget() const
{
    return nullptr;
}

bool AudioPreviewPlugin::showToolBar() const
{
    return true;
}
