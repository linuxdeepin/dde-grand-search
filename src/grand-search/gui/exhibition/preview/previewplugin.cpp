/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     houchengqiu<houchengqiu@uniontech.com>
 *
 * Maintainer: houchengqiu<houchengqiu@uniontech.com>
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
#include "previewplugin.h"
#include "utils/utils.h"

using namespace GrandSearch;

PreviewPlugin::PreviewPlugin(QObject *parent)
    : QObject(parent)
{

}

PreviewPlugin::~PreviewPlugin()
{

}

bool PreviewPlugin::stopPreview() const
{

}

QWidget *PreviewPlugin::statusBarWidget() const
{
    return nullptr;
}

QWidget *PreviewPlugin::toolBarWidget() const
{
    return nullptr;
}

bool PreviewPlugin::showStatusBar() const
{
    return false;
}

bool PreviewPlugin::showToolBar() const
{
    return false;
}
