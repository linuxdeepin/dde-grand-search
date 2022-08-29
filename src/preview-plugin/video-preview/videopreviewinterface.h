// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VIDEOPREVIEWINTERFACE_H
#define VIDEOPREVIEWINTERFACE_H

#include "libvideoviewer.h"

#include <previewplugininterface.h>

#include <QSharedPointer>

class VideoPreviewInterface : public QObject, public GrandSearch::PreviewPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(GrandSearch::PreviewPluginInterface)
    Q_PLUGIN_METADATA(IID FilePreviewInterface_iid)
public:
    explicit VideoPreviewInterface(QObject *parent = nullptr);
    virtual GrandSearch::PreviewPlugin *create(const QString &mimetype);
protected:
    QSharedPointer<GrandSearch::LibVideoViewer> lib;
};

#endif // VIDEOPREVIEWINTERFACE_H
