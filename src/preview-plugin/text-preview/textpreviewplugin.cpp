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

#include "textpreview_global.h"
#include "textpreviewplugin.h"
#include "textview.h"

#include <QFileInfo>
#include <QDateTime>

GRANDSEARCH_USE_NAMESPACE
using namespace GrandSearch::text_preview;

TextPreviewPlugin::TextPreviewPlugin(QObject *parent)
    : QObject(parent)
    , PreviewPlugin()
{

}

TextPreviewPlugin::~TextPreviewPlugin()
{
    delete m_view;
}

void TextPreviewPlugin::init(QObject *proxyInter)
{
    Q_UNUSED(proxyInter)

    if (!m_view) {
        m_view = new TextView();
        m_view->initUI();
    }
}

bool TextPreviewPlugin::previewItem(const ItemInfo &item)
{
    const QString path = item.value(PREVIEW_ITEMINFO_ITEM);
    if (path.isEmpty())
        return false;

    m_item = item;
    m_view->setSource(path);
    return true;
}

ItemInfo TextPreviewPlugin::item() const
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


DetailInfoList TextPreviewPlugin::getAttributeDetailInfo() const
{
    return {};
}
