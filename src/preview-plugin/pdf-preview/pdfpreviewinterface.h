// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PDFPREVIEWINTERFACE_H
#define PDFPREVIEWINTERFACE_H

#include "previewplugininterface.h"

class PDFPreviewInterface : public QObject, public GrandSearch::PreviewPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(GrandSearch::PreviewPluginInterface)
    Q_PLUGIN_METADATA(IID FilePreviewInterface_iid)
public:
    explicit PDFPreviewInterface(QObject *parent = nullptr);
    virtual GrandSearch::PreviewPlugin *create(const QString &mimetype);
};

#endif // PDFPREVIEWINTERFACE_H
