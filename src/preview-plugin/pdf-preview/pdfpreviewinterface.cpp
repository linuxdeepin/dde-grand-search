// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pdfpreviewinterface.h"
#include "pdfpreviewplugin.h"

PDFPreviewInterface::PDFPreviewInterface(QObject *parent)
    : QObject(parent)
    , GrandSearch::PreviewPluginInterface()
{

}

GrandSearch::PreviewPlugin *PDFPreviewInterface::create(const QString &mimetype)
{
    Q_UNUSED(mimetype)

    return new PDFPreviewPlugin();
}
