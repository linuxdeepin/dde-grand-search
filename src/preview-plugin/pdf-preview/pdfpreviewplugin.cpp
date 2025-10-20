// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pdfpreview_global.h"
#include "pdfpreviewplugin.h"
#include "pdfview.h"

#include <QFileInfo>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(logPdfPreview, "org.deepin.dde.grandsearch.plugin.pdf")
GRANDSEARCH_USE_NAMESPACE
using namespace GrandSearch::pdf_preview;

PDFPreviewPlugin::PDFPreviewPlugin(QObject *parent)
    : QObject (parent)
    , PreviewPlugin()
{
    qCDebug(logPdfPreview) << "PDFPreviewPlugin created";
}

PDFPreviewPlugin::~PDFPreviewPlugin()
{
    qCDebug(logPdfPreview) << "PDFPreviewPlugin destroyed";
    if (m_pdfView)
        m_pdfView->deleteLater();
}

void PDFPreviewPlugin::init(QObject *proxyInter)
{
    Q_UNUSED(proxyInter)
    qCDebug(logPdfPreview) << "PDFPreviewPlugin initialized";
}

bool PDFPreviewPlugin::previewItem(const ItemInfo &item)
{
    const QString path = item.value(PREVIEW_ITEMINFO_ITEM);
    if (path.isEmpty()) {
        qCWarning(logPdfPreview) << "PDF file path is empty - Cannot preview";
        return false;
    }

    qCDebug(logPdfPreview) << "Previewing PDF file - Path:" << path;

    QFileInfo fileInfo(path);
    if (!fileInfo.isReadable()) {
        qCWarning(logPdfPreview) << "PDF file is not readable:" << path;
        return false;
    }

    if (!m_pdfView) {
        m_pdfView = new PDFView(path);
        qCDebug(logPdfPreview) << "PDFView created";
    }

    m_item = item;
    qCDebug(logPdfPreview) << "PDF preview completed successfully - Path:" << path;
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
    qCDebug(logPdfPreview) << "Stopping PDF preview";
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
