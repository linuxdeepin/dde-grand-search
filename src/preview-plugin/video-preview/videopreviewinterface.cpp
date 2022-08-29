// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "videopreviewinterface.h"
#include "videopreviewplugin.h"

VideoPreviewInterface::VideoPreviewInterface(QObject *parent)
  : QObject(parent)
  , GrandSearch::PreviewPluginInterface()
{
    lib.reset(new GrandSearch::LibVideoViewer);
    lib->initLibrary();
}

GrandSearch::PreviewPlugin *VideoPreviewInterface::create(const QString &mimetype)
{
    auto plugin = new VideoPreviewPlugin();
    plugin->setParser(lib);
    return plugin;
}
