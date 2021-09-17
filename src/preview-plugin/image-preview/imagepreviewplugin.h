/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#ifndef IMAGEPREVIEWPLUGIN_H
#define IMAGEPREVIEWPLUGIN_H

#include "previewplugin.h"

class ImageView;
class ImagePreviewPlugin : public QObject, public GrandSearch::PreviewPlugin
{
    Q_OBJECT
public:
    explicit ImagePreviewPlugin(QObject *parent = nullptr);
    ~ImagePreviewPlugin() Q_DECL_OVERRIDE;
    void init(QObject *proxyInter) Q_DECL_OVERRIDE;
    bool previewItem(const GrandSearch::ItemInfo &item) Q_DECL_OVERRIDE;
    GrandSearch::ItemInfo item() const Q_DECL_OVERRIDE;
    QWidget *contentWidget() const Q_DECL_OVERRIDE;
    bool stopPreview() Q_DECL_OVERRIDE;
    GrandSearch::DetailInfoList getAttributeDetailInfo() const Q_DECL_OVERRIDE;
    QWidget *toolBarWidget() const Q_DECL_OVERRIDE;
    bool showToolBar() const Q_DECL_OVERRIDE;
private:
    GrandSearch::ItemInfo m_item;
    ImageView *m_imageView = nullptr;
    GrandSearch::DetailInfoList m_detailInfos;
};

#endif // IMAGEPREVIEWPLUGIN_H
