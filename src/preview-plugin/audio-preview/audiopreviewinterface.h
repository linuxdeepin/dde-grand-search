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
