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
#include "imagepreviewplugin.h"
#include "imageview.h"
#include "global/commontools.h"

#include <QFileInfo>
#include <QDateTime>

using namespace GrandSearch;

ImagePreviewPlugin::ImagePreviewPlugin(QObject *parent)
    : QObject (parent)
    , GrandSearch::PreviewPlugin()
{

}

ImagePreviewPlugin::~ImagePreviewPlugin()
{
    if (m_imageView)
        m_imageView->deleteLater();
}

void ImagePreviewPlugin::init(QObject *proxyInter)
{
    Q_UNUSED(proxyInter)
}

bool ImagePreviewPlugin::previewItem(const GrandSearch::ItemInfo &item)
{
    const QString path = item.value(PREVIEW_ITEMINFO_ITEM);

    if (path.isEmpty())
        return false;

    const QString type = item.value(PREVIEW_ITEMINFO_TYPE);
    if (!m_imageView)
        m_imageView = new ImageView(path, type);

    // 尺寸
    auto dimension = m_imageView->sourceSize();
    QString dimensionStr("--");
    if (dimension.isValid())
        dimensionStr = QString("%1*%2").arg(dimension.width()).arg(dimension.height());

    DetailTagInfo tagInfos;
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Dimension:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    DetailContentInfo contentInfos;
    contentInfos.insert(DetailInfoProperty::Text, QVariant(dimensionStr));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideMiddle));

    DetailInfo detailInfo = qMakePair(tagInfos, contentInfos);
    m_detailInfos.push_back(detailInfo);

    QFileInfo fileInfo(path);

    // 类型
    auto suffix = fileInfo.suffix();

    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Type:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(suffix.isEmpty() ? "--" : suffix.toUpper()));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_detailInfos.push_back(detailInfo);

    // 大小
    auto size = fileInfo.size();
    auto fileSize = qMakePair<QString, QString>("Size:", GrandSearch::CommonTools::formatFileSize(size));

    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Size:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(GrandSearch::CommonTools::formatFileSize(size)));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_detailInfos.push_back(detailInfo);

    // 位置
    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Location:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(fileInfo.absoluteFilePath()));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_detailInfos.push_back(detailInfo);

    // 修改时间
    auto time = fileInfo.lastModified();

    tagInfos.clear();
    tagInfos.insert(DetailInfoProperty::Text, QVariant(tr("Time modified:")));
    tagInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideNone));

    contentInfos.clear();
    contentInfos.insert(DetailInfoProperty::Text, QVariant(time.toString(GrandSearch::CommonTools::dateTimeFormat())));
    contentInfos.insert(DetailInfoProperty::ElideMode, QVariant::fromValue(Qt::ElideRight));

    detailInfo = qMakePair(tagInfos, contentInfos);
    m_detailInfos.push_back(detailInfo);

    m_item = item;
    return true;
}

GrandSearch::ItemInfo ImagePreviewPlugin::item() const
{
    return m_item;
}

QWidget *ImagePreviewPlugin::contentWidget() const
{
    return m_imageView;
}

bool ImagePreviewPlugin::stopPreview()
{
    return m_imageView->stopPreview();
}

GrandSearch::DetailInfoList ImagePreviewPlugin::getAttributeDetailInfo() const
{
    return m_detailInfos;
}

QWidget *ImagePreviewPlugin::toolBarWidget() const
{
    return nullptr;
}

bool ImagePreviewPlugin::showToolBar() const
{
    return true;
}
