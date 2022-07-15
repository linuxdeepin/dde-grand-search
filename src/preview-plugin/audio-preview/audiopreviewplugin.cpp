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
#include <QUrl>

using namespace GrandSearch;

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

    // 歌手 尾部截断
    DetailTagInfo tagInfos;
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Artist:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    DetailContentInfo contentInfos;
    contentInfos.insert(DetailInfoProperty::Text, QVariant(amd.artist.isEmpty() ? "--" : amd.artist));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    DetailInfo detailInfo = qMakePair(tagInfos, contentInfos);
    m_detailInfos.push_back(detailInfo);

    // 专辑 尾部截断
    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Album:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(amd.album.isEmpty() ? "--" : amd.album));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_detailInfos.push_back(detailInfo);

    // 时长
    if (amd.duration.isEmpty() && m_parser.get()) {
        if (m_parser->initLibrary()) {
            qint64 dura = -1;
            m_parser->getDuration(QUrl::fromLocalFile(path), dura);
            if (dura >= 0)
                amd.duration = GrandSearch::CommonTools::durationString(dura);
        }
    }

    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Duration:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(amd.duration.isEmpty() ? "--" : amd.duration));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_detailInfos.push_back(detailInfo);

    // 类型
    QFileInfo fileInfo(path);
    auto suffix = fileInfo.suffix();

    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Type:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(suffix.isEmpty() ? "--" : suffix));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_detailInfos.push_back(detailInfo);

    // 位置
    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Location:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(fileInfo.absoluteFilePath()));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideMiddle));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_detailInfos.push_back(detailInfo);

    // 修改时间
    auto time = fileInfo.lastModified();

    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Time modified:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(time.toString(GrandSearch::CommonTools::dateTimeFormat())));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideMiddle));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_detailInfos.push_back(detailInfo);

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

void AudioPreviewPlugin::setExtendParser(QSharedPointer<LibAudioViewer> parser)
{
    m_parser = parser;
}
