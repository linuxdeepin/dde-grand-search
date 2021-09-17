/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
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
#include "pluginproxy.h"
#include "detailinfowidget.h"

#include <QDebug>

PluginProxy::PluginProxy(PreviewWidget *parent)
    : QObject(parent)
    , q(parent)
{

}

void PluginProxy::updateDetailInfo(GrandSearch::PreviewPlugin *plugin)
{
    if (plugin == nullptr || plugin != q->m_preview) {
        qWarning() << "updateDetailInfo: invaild plugin.";
        return;
    }

    if (thread() != qApp->thread()) {
        qWarning() << __FUNCTION__ << "could not be called in other thread.";
        return;
    }

    q->m_detailInfoWidget->setDetailInfo(plugin->getAttributeDetailInfo());
}
