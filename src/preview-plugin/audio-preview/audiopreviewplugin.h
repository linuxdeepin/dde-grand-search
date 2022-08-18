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
#ifndef AUDIOPREVIEWPLUGIN_H
#define AUDIOPREVIEWPLUGIN_H

#include "previewplugin.h"
#include "libaudioviewer.h"

#include <QSharedPointer>

class AudioView;
class AudioPreviewPlugin : public QObject, public GrandSearch::PreviewPlugin
{
    Q_OBJECT
public:
    explicit AudioPreviewPlugin(QObject *parent = nullptr);
    ~AudioPreviewPlugin() Q_DECL_OVERRIDE;
    void init(QObject *proxyInter) Q_DECL_OVERRIDE;
    bool previewItem(const GrandSearch::ItemInfo &item) Q_DECL_OVERRIDE;
    GrandSearch::ItemInfo item() const Q_DECL_OVERRIDE;
    bool stopPreview() Q_DECL_OVERRIDE;
    QWidget *contentWidget() const Q_DECL_OVERRIDE;
    GrandSearch::DetailInfoList getAttributeDetailInfo() const Q_DECL_OVERRIDE;
    QWidget *toolBarWidget() const Q_DECL_OVERRIDE;
    bool showToolBar() const Q_DECL_OVERRIDE;
    void setExtendParser(QSharedPointer<GrandSearch::LibAudioViewer> parser);
private:
    GrandSearch::ItemInfo m_item;
    GrandSearch::DetailInfoList m_detailInfos;
    AudioView *m_audioView = nullptr;
    QSharedPointer<GrandSearch::LibAudioViewer> m_parser;
};

#endif // AUDIOPREVIEWPLUGIN_H
