// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imagepreviewinterface.h"
#include "imagepreviewplugin.h"

ImagePreviewInterface::ImagePreviewInterface(QObject *parent)
    : QObject(parent)
    , GrandSearch::PreviewPluginInterface()
{

}

GrandSearch::PreviewPlugin *ImagePreviewInterface::create(const QString &mimetype)
{
    Q_UNUSED(mimetype)

    return new ImagePreviewPlugin();
}
