// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

void TextPreviewPlugin::init(QObject *proxyInter)
{
    Q_UNUSED(proxyInter)

    if (!m_view) {
        m_view = new TextView();
        m_view->initUI();
    }
}

bool TextPreviewPlugin::previewItem(const GrandSearch::ItemInfo &item)
{
    const QString path = item.value(PREVIEW_ITEMINFO_ITEM);
    if (path.isEmpty())
        return false;

    m_item = item;
    m_view->setSource(path);
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


GrandSearch::DetailInfoList TextPreviewPlugin::getAttributeDetailInfo() const
{
    return {};
}
