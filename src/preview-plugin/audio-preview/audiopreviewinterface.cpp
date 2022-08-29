// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "audiopreviewinterface.h"
#include "audiopreviewplugin.h"

AudioPreviewInterface::AudioPreviewInterface(QObject *parent)
    : QObject(parent)
    , GrandSearch::PreviewPluginInterface()
{
    lib.reset(new GrandSearch::LibAudioViewer);
}

GrandSearch::PreviewPlugin *AudioPreviewInterface::create(const QString &mimetype)
{
    Q_UNUSED(mimetype)
    auto plugin = new AudioPreviewPlugin();
    plugin->setExtendParser(lib);
    return plugin;
}
