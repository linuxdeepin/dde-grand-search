// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textpreview_global.h"
#include "textpreviewplugin.h"
#include "textview.h"

#include <QFileInfo>
#include <QDateTime>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(logTextPreview, "org.deepin.dde.grandsearch.plugin.text")

GRANDSEARCH_USE_NAMESPACE
using namespace GrandSearch::text_preview;

TextPreviewPlugin::TextPreviewPlugin(QObject *parent)
    : QObject(parent)
    , PreviewPlugin()
{
    qCDebug(logTextPreview) << "TextPreviewPlugin created";
}

TextPreviewPlugin::~TextPreviewPlugin()
{
    qCDebug(logTextPreview) << "TextPreviewPlugin destroyed";
    delete m_view;
}

void TextPreviewPlugin::init(QObject *proxyInter)
{
    Q_UNUSED(proxyInter)
    qCDebug(logTextPreview) << "Initializing TextPreviewPlugin";

    if (!m_view) {
        m_view = new TextView();
        m_view->initUI();
        qCDebug(logTextPreview) << "TextView created and initialized";
    }
}

bool TextPreviewPlugin::previewItem(const ItemInfo &item)
{
    const QString path = item.value(PREVIEW_ITEMINFO_ITEM);
    if (path.isEmpty()) {
        qCWarning(logTextPreview) << "Text file path is empty - Cannot preview";
        return false;
    }
    qCDebug(logTextPreview) << "Previewing text file - Path:" << path;

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
    qCDebug(logTextPreview) << "Stopping text preview";
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
