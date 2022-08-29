// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTPREVIEWINTERFACE_H
#define TEXTPREVIEWINTERFACE_H

#include <previewplugininterface.h>

class TextPreviewInterface : public QObject, public GrandSearch::PreviewPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(GrandSearch::PreviewPluginInterface)
    Q_PLUGIN_METADATA(IID FilePreviewInterface_iid)
public:
    explicit TextPreviewInterface(QObject *parent = 0);
    virtual GrandSearch::PreviewPlugin *create(const QString &mimetype);
};

#endif // TEXTPREVIEWINTERFACE_H
