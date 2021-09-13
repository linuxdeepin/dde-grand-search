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
#include "textpreviewplugin.h"
#include "textview.h"

#include <QFileInfo>
#include <QDateTime>

TextPreviewPlugin::TextPreviewPlugin(QObject *parent)
    : QObject(parent)
    , GrandSearch::PreviewPlugin()
{

}

TextPreviewPlugin::~TextPreviewPlugin()
{
    delete m_view;
}

bool TextPreviewPlugin::previewItem(const GrandSearch::ItemInfo &item)
{
    const QString path = item.value(PREVIEW_ITEMINFO_ITEM);
    if (path.isEmpty())
        return false;

    QFileInfo fileInfo(path);
    if (!fileInfo.isReadable())
        return false;

    m_detailInfo.clear();
    m_detailInfo.append(qMakePair(QString("size"), QString::number(fileInfo.size())));
    m_detailInfo.append(qMakePair(QString("modify time"), fileInfo.lastModified().toString()));
    m_detailInfo.append(qMakePair(QString("path"), fileInfo.absoluteFilePath()));

    if (!m_view)
        m_view = new TextView();

    m_view->setSource(path);
    m_item = item;

    return true;
}

GrandSearch::ItemInfo TextPreviewPlugin::item() const
{
    return m_item;
}

QWidget *TextPreviewPlugin::contentWidget() const
{
    return m_view;
}

bool TextPreviewPlugin::stopPreview()
{
    return true;
}

QWidget *TextPreviewPlugin::toolBarWidget() const
{
    return nullptr;
}

bool TextPreviewPlugin::showToolBar() const
{
    return true;
}

QRect TextPreviewPlugin::getValidClickRegion() const
{
    return QRect();
}

GrandSearch::DetailInfoList TextPreviewPlugin::getAttributeDetailInfo() const
{
    return {};//m_detailInfo;
}
