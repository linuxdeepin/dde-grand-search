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

#include <DApplication>

#include <QGSettings>

#define GrandSearchPlugin "grand-search"
#define MenuOpenSetting "menu_open_setting"
#define GrandSearchApp "dde-grand-search"

#define SchemaId    "com.deepin.dde.dock.module.grand-search"
#define SchemaPath  "/com/deepin/dde/dock/module/grand-search/"
#define SchemaKeyMenuEnable "menuEnable"

DWIDGET_USE_NAMESPACE

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
    // 加载翻译
    QString appName = qApp->applicationName();
    qApp->setApplicationName(GrandSearchApp);
    qApp->loadTranslator();
    qApp->setApplicationName(appName);

    m_proxyInter = proxyInter;

    if (m_searchWidget.isNull())
        m_searchWidget.reset(new GrandSearchWidget);

    // 如果插件没有被禁用则在初始化插件时才添加主控件到面板上
    if (!pluginIsDisable()) {
        m_proxyInter->itemAdded(this, pluginName());
    }

    m_gsettings.reset(new QGSettings(SchemaId, SchemaPath));

    connect(m_gsettings.data(), &QGSettings::changed, this, &DdeGrandSearchDockPlugin::onGsettingsChanged);
}

QWidget *DdeGrandSearchDockPlugin::itemWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);
    return m_searchWidget.data();
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
        return m_searchWidget->itemCommand(itemKey);

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

const QString DdeGrandSearchDockPlugin::itemContextMenu(const QString &itemKey)
{
    if (itemKey != GrandSearchPlugin) {
        return QString();
    }

    QList<QVariant> items;
    items.reserve(1);

    QMap<QString, QVariant> item;
    item["itemId"] = MenuOpenSetting;
    item["itemText"] = tr("Search settings");
    item["isActive"] = true;
    items.push_back(item);

    QMap<QString, QVariant> menu;
    menu["items"] = items;
    menu["checkableMenu"] = false;
    menu["singleCheck"] = false;

    return QJsonDocument::fromVariant(menu).toJson();
}

void DdeGrandSearchDockPlugin::invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked)
{
    Q_UNUSED(checked)

    if (itemKey != GrandSearchPlugin)
        return;

    if (menuId == MenuOpenSetting) {
        QProcess::startDetached("dde-grand-search", QStringList() << "--setting");
    }
}

void DdeGrandSearchDockPlugin::onGsettingsChanged(const QString &key)
{
    Q_ASSERT(m_gsettings);

    qDebug() << "gsettings changed,and key:" << key << "    value:" << m_gsettings->get(key);
    if (key == SchemaKeyMenuEnable) {
        bool enable = m_gsettings->get(key).toBool();
        qInfo() << "The status of whether the grand search right-click menu is enabled changes to:" << enable;
    }
}

