// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginproxy.h"
#include "generalwidget/detailwidget.h"

#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

using namespace GrandSearch;

PluginProxy::PluginProxy(PreviewWidget *parent)
    : QObject(parent)
    , q(parent)
{

}

void PluginProxy::updateDetailInfo(PreviewPlugin *plugin)
{
    if (plugin == nullptr || plugin != q->m_preview) {
        qCWarning(logGrandSearch) << "Invalid plugin provided to updateDetailInfo - Plugin is null or not the current preview plugin";
        return;
    }

    if (thread() != qApp->thread()) {
        qCWarning(logGrandSearch) << "Thread violation - updateDetailInfo must be called from the main thread";
        return;
    }

    q->m_detailInfoWidget->setDetailInfoList(plugin->getAttributeDetailInfo());
}
