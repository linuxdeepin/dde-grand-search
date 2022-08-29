// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef AUDIOPREVIEWINTERFACE_H
#define AUDIOPREVIEWINTERFACE_H

#include "previewplugininterface.h"
#include "libaudioviewer.h"

#include <QSharedPointer>

class AudioPreviewInterface : public QObject, public GrandSearch::PreviewPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(GrandSearch::PreviewPluginInterface)
    Q_PLUGIN_METADATA(IID FilePreviewInterface_iid)
public:
    explicit AudioPreviewInterface(QObject *parent = nullptr);
    virtual GrandSearch::PreviewPlugin *create(const QString &mimetype);
protected:
    QSharedPointer<GrandSearch::LibAudioViewer> lib;
};

#endif // AUDIOPREVIEWINTERFACE_H
