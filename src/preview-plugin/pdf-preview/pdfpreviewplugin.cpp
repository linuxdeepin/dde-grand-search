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
#include "pdfpreview_global.h"
#include "pdfpreviewplugin.h"
#include "pdfview.h"

#include <QFileInfo>

GRANDSEARCH_USE_NAMESPACE
using namespace GrandSearch::pdf_preview;

PDFPreviewPlugin::PDFPreviewPlugin(QObject *parent)
    : QObject (parent)
    , PreviewPlugin()
{

}

PDFPreviewPlugin::~PDFPreviewPlugin()
{
    if (m_pdfView)
        m_pdfView->deleteLater();
}

void PDFPreviewPlugin::init(QObject *proxyInter)
{
    Q_UNUSED(proxyInter)
}

bool PDFPreviewPlugin::previewItem(const ItemInfo &item)
{
    const QString path = item.value(PREVIEW_ITEMINFO_ITEM);
    if (path.isEmpty())
        return false;

    QFileInfo fileInfo(path);
    if (!fileInfo.isReadable())
        return false;

    if (!m_pdfView)
        m_pdfView = new PDFView(path);

    m_item = item;
    return true;
}

ItemInfo PDFPreviewPlugin::item() const
{
    return m_item;
}

QWidget *PDFPreviewPlugin::contentWidget() const
{
    return m_pdfView;
}

bool PDFPreviewPlugin::stopPreview()
{
    return true;
}

DetailInfoList PDFPreviewPlugin::getAttributeDetailInfo() const
{
    return {};
}

QWidget *PDFPreviewPlugin::toolBarWidget() const
{
    return nullptr;
}

bool PDFPreviewPlugin::showToolBar() const
{
    return true;
}
