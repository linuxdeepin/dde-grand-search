// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "textpreviewinterface.h"
#include "textpreviewplugin.h"

TextPreviewInterface::TextPreviewInterface(QObject *parent)
    : QObject(parent)
    , GrandSearch::PreviewPluginInterface()
{

}

GrandSearch::PreviewPlugin *TextPreviewInterface::create(const QString &mimetype)
{
    return new TextPreviewPlugin();
}
