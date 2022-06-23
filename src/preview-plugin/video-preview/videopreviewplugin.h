/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#ifndef VIDEOPREVIEWPLUGIN_H
#define VIDEOPREVIEWPLUGIN_H

#include <previewplugin.h>

#include <QFuture>

namespace GrandSearch {
namespace video_preview {

class DecodeBridge : public QObject
{
    Q_OBJECT
public:
    DecodeBridge();
    ~DecodeBridge();
    static QVariantHash decode(QSharedPointer<DecodeBridge> self, const QString &file);
    static QPixmap scaleAndRound(const QImage &img, const QSize &size);
signals:
    void sigUpdateInfo(const QVariantHash &, bool needUpdate);
public:
    volatile bool decoding = false;
};

class VideoView;
class VideoPreviewPlugin : public QObject, public PreviewPlugin
{
    Q_OBJECT
public:
    explicit VideoPreviewPlugin(QObject *parent = nullptr);
    ~VideoPreviewPlugin() Q_DECL_OVERRIDE;
    void init(QObject *proxyInter) Q_DECL_OVERRIDE;
    bool previewItem(const ItemInfo &item) Q_DECL_OVERRIDE;
    ItemInfo item() const Q_DECL_OVERRIDE;
    QWidget *contentWidget() const Q_DECL_OVERRIDE;
    bool stopPreview() Q_DECL_OVERRIDE;
    QWidget *toolBarWidget() const Q_DECL_OVERRIDE;
    bool showToolBar() const Q_DECL_OVERRIDE;
    DetailInfoList getAttributeDetailInfo() const Q_DECL_OVERRIDE;
protected slots:
    void updateInfo(const QVariantHash &, bool needUpdate);
protected:
    ItemInfo m_item;
    DetailInfoList m_infos;
    VideoView *m_view = nullptr;
    QObject *m_proxy = nullptr;
    QSharedPointer<DecodeBridge> m_decode;
};

}}

#endif // VIDEOPREVIEWPLUGIN_H
