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
