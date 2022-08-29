// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEPREVIEWINTERFACE_H
#define IMAGEPREVIEWINTERFACE_H

#include "previewplugininterface.h"

class ImagePreviewInterface : public QObject, public GrandSearch::PreviewPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(GrandSearch::PreviewPluginInterface)
    Q_PLUGIN_METADATA(IID FilePreviewInterface_iid)
public:
    explicit ImagePreviewInterface(QObject *parent = nullptr);
    virtual GrandSearch::PreviewPlugin *create(const QString &mimetype);
};

#endif // IMAGEPREVIEWINTERFACE_H
