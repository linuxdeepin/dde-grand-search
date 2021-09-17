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

bool ImagePreviewPlugin::previewItem(const GrandSearch::ItemInfo &item)
{
    const QString path = item.value(PREVIEW_ITEMINFO_ITEM);

    if (path.isEmpty())
        return false;

    QFileInfo fileInfo(path);
    if (!fileInfo.isReadable())
        return false;

    const QString type = item.value(PREVIEW_ITEMINFO_TYPE);
    if (!m_imageView)
        m_imageView = new ImageView(path, type);

    // 尺寸
    auto dimension = m_imageView->sourceSize();
    QString dimensionStr("--");
    if (dimension.isValid())
        dimensionStr = QString("%1*%2").arg(dimension.width()).arg(dimension.height());
    auto fileDimension = qMakePair<QString, QString>("Dimension:", dimensionStr);
    m_detailInfos.push_back(fileDimension);

    // 类型
    auto suffix = fileInfo.suffix();
    auto fileType = qMakePair<QString, QString>("Type:", suffix.isEmpty() ? "--" : suffix.toUpper());
    m_detailInfos.push_back(fileType);

    // 大小
    auto size = fileInfo.size();
    auto fileSize = qMakePair<QString, QString>("Size:", GrandSearch::CommonTools::formatFileSize(size));
    m_detailInfos.push_back(fileSize);

    // 位置
    auto location = qMakePair<QString, QString>("Location:", fileInfo.absoluteFilePath());
    m_detailInfos.push_back(location);

    // 修改时间
    auto time = fileInfo.lastModified();
    auto timeModified = qMakePair<QString, QString>("Time modified:", time.toString(GrandSearch::CommonTools::dateTimeFormat()));
    m_detailInfos.push_back(timeModified);

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
