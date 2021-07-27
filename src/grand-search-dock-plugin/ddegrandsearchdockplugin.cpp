/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
#include "ddegrandsearchdockplugin.h"
#include "gui/grandsearchwidget.h"

const QString GrandSearchPlugin = QStringLiteral("ddegrandsearch_dockplugin");

DdeGrandSearchDockPlugin::DdeGrandSearchDockPlugin(QObject *parent)
    : QObject (parent)
    , m_searchWidget(nullptr)
{

}

DdeGrandSearchDockPlugin::~DdeGrandSearchDockPlugin()
{

}

const QString DdeGrandSearchDockPlugin::pluginName() const
{
    return GrandSearchPlugin;
}

void DdeGrandSearchDockPlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInter = proxyInter;

    if (m_searchWidget.isNull())
        m_searchWidget.reset(new GrandSearchWidget);

    // 如果插件没有被禁用则在初始化插件时才添加主控件到面板上
    if (!pluginIsDisable()) {
        m_proxyInter->itemAdded(this, pluginName());
    }
}

QWidget *DdeGrandSearchDockPlugin::itemWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);
    return m_searchWidget.data();
}

void DdeGrandSearchDockPlugin::refreshIcon(const QString &itemKey)
{
    Q_UNUSED(itemKey);
    Q_ASSERT(m_searchWidget.data());

    m_searchWidget->updateIcon();
}

bool DdeGrandSearchDockPlugin::pluginIsAllowDisable()
{
    // 插件不允许禁用
    return false;
}

bool DdeGrandSearchDockPlugin::pluginIsDisable()
{
    // 第二个参数 “disabled” 表示存储这个值的键（所有配置都是以键值对的方式存储）
    // 第三个参数表示默认值，即默认不禁用
    return m_proxyInter->getValue(this, "disabled", false).toBool();
}

void DdeGrandSearchDockPlugin::pluginStateSwitched()
{
    // 获取当前禁用状态的反值作为新的状态值
    const bool disabledNew = !pluginIsDisable();
    // 存储新的状态值
    m_proxyInter->saveValue(this, "disabled", disabledNew);

    // 根据新的禁用状态值处理主控件的加载和卸载
    if (disabledNew) {
        m_proxyInter->itemRemoved(this, pluginName());
    } else {
        m_proxyInter->itemAdded(this, pluginName());
    }
}

const QString DdeGrandSearchDockPlugin::itemCommand(const QString &itemKey)
{
    if (GrandSearchPlugin == itemKey)
        return QString("dbus-send --print-reply --dest=com.deepin.dde.GrandSearch /com/deepin/dde/GrandSearch com.deepin.dde.GrandSearch.SetVisible boolean:true");

    return QString();
}

int DdeGrandSearchDockPlugin::itemSortKey(const QString &itemKey)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    return m_proxyInter->getValue(this, key, 0).toInt();
}

void DdeGrandSearchDockPlugin::setSortKey(const QString &itemKey, const int order)
{
    const QString key = QString("pos_%1_%2").arg(itemKey).arg(Dock::Efficient);
    m_proxyInter->saveValue(this, key, order);
}

