// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddegrandsearchdockplugin.h"
#include "gui/grandsearchwidget.h"
#include "gui/tipswidget.h"

#include <DApplication>

#if (QT_VERSION_MAJOR < 6)
#include <QGSettings>
#endif
#include <QLabel>
#include <QLoggingCategory>

#define GrandSearchPlugin "grand-search"
#define MenuOpenSetting "menu_open_setting"
#define GrandSearchApp "dde-grand-search"

#define SchemaId "com.deepin.dde.dock.module.grand-search"
#define SchemaPath "/com/deepin/dde/dock/module/grand-search/"
#define SchemaKeyMenuEnable "menuEnable"

Q_LOGGING_CATEGORY(logDock, "org.deepin.dde.dock.plugin.grandsearch")
using namespace GrandSearch;
DWIDGET_USE_NAMESPACE

#define QUICK_ITEM_KEY QStringLiteral("quick_item_key")

DdeGrandSearchDockPlugin::DdeGrandSearchDockPlugin(QObject *parent)
    : QObject(parent), m_searchWidget(nullptr)
{
    qCDebug(logDock) << "Grand Search dock plugin constructor called";
}

DdeGrandSearchDockPlugin::~DdeGrandSearchDockPlugin()
{
    qCDebug(logDock) << "Grand Search dock plugin destructor called";
}

const QString DdeGrandSearchDockPlugin::pluginName() const
{
    return GrandSearchPlugin;
}

const QString DdeGrandSearchDockPlugin::pluginDisplayName() const
{
    return tr("Grand Search");
}

void DdeGrandSearchDockPlugin::init(PluginProxyInterface *proxyInter)
{
    qCDebug(logDock) << "Initializing Grand Search dock plugin";

    // 加载翻译
    QString appName = qApp->applicationName();
    qApp->setApplicationName(GrandSearchApp);
    qApp->loadTranslator();
    qApp->setApplicationName(appName);
    qCDebug(logDock) << "Translation loaded for Grand Search dock plugin";

    m_proxyInter = proxyInter;

    if (m_searchWidget.isNull()) {
        m_searchWidget.reset(new GrandSearchWidget);
        connect(m_searchWidget.data(), &GrandSearchWidget::visibleChanged, this,
                &DdeGrandSearchDockPlugin::onVisibleChanged);
        qCDebug(logDock) << "Grand Search widget created and connected";
    }
    if (m_tipsWidget.isNull()) {
        m_tipsWidget.reset(new TipsWidget);
        qCDebug(logDock) << "Tips widget created";
    }

#ifdef USE_DOCK_2_0
    if (m_quickWidget.isNull()) {
        m_quickWidget.reset(new QuickPanel(pluginDisplayName()));
        qCDebug(logDock) << "Quick panel widget created for dock 2.0";
    }
#endif

    // 如果插件没有被禁用则在初始化插件时才添加主控件到面板上
    if (!pluginIsDisable()) {
        m_proxyInter->itemAdded(this, pluginName());
    }
#if (QT_VERSION_MAJOR < 6)
    if (QGSettings::isSchemaInstalled(SchemaId)) {
        m_gsettings.reset(new QGSettings(SchemaId, SchemaPath));
        connect(m_gsettings.data(), &QGSettings::changed, this, &DdeGrandSearchDockPlugin::onGsettingsChanged);
        qCDebug(logDock) << "GSettings initialized for Grand Search dock plugin - Schema:" << SchemaId;
    } else {
        qCWarning(logDock) << "GSettings schema not found - Schema:" << SchemaId;
    }
#endif

    qCDebug(logDock) << "Grand Search dock plugin initialization completed";
}

QWidget *DdeGrandSearchDockPlugin::itemWidget(const QString &itemKey)
{
    qCDebug(logDock) << "Requesting item widget for key:" << itemKey;

    if (itemKey == QUICK_ITEM_KEY) {
        qCDebug(logDock) << "Returning quick panel widget";
        return m_quickWidget.data();
    } else if (itemKey == GrandSearchPlugin) {
        qCDebug(logDock) << "Returning grand search widget";
        return m_searchWidget.data();
    }

    qCWarning(logDock) << "Unknown item key requested:" << itemKey;
    return nullptr;
}

QWidget *DdeGrandSearchDockPlugin::itemTipsWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);
    qCDebug(logDock) << "Setting up tips widget for item:" << itemKey;

    // 设置/刷新 tips 中的信息
    m_tipsWidget->setText(tr("Grand Search"));
    return m_tipsWidget.data();
}

bool DdeGrandSearchDockPlugin::pluginIsAllowDisable()
{
    // 插件允许禁用
    qCDebug(logDock) << "Grand Search plugin allows disable: true";
    return true;
}

bool DdeGrandSearchDockPlugin::pluginIsDisable()
{
    // 第二个参数 "disabled" 表示存储这个值的键（所有配置都是以键值对的方式存储）
    // 第三个参数表示默认值，即默认不禁用
    return m_proxyInter->getValue(this, "disabled", false).toBool();
}

void DdeGrandSearchDockPlugin::pluginStateSwitched()
{
    qCDebug(logDock) << "Grand Search plugin state switching triggered";

    // 获取当前禁用状态的反值作为新的状态值
    const bool disabledNew = !pluginIsDisable();
    qCDebug(logDock) << "Switching Grand Search plugin state - New disabled state:" << disabledNew;

    // 存储新的状态值
    m_proxyInter->saveValue(this, "disabled", disabledNew);

    // 根据新的禁用状态值处理主控件的加载和卸载
    if (disabledNew) {
        m_proxyInter->itemRemoved(this, pluginName());
        qCDebug(logDock) << "Grand Search plugin item removed from dock - Plugin disabled";
    } else {
        m_proxyInter->itemAdded(this, pluginName());
        qCDebug(logDock) << "Grand Search plugin item added to dock - Plugin enabled";
    }
}

const QString DdeGrandSearchDockPlugin::itemCommand(const QString &itemKey)
{
    qCDebug(logDock) << "Executing item command for key:" << itemKey;

    if (GrandSearchPlugin == itemKey || itemKey == QUICK_ITEM_KEY) {
        m_proxyInter->requestSetAppletVisible(this, pluginName(), false);
        return m_searchWidget->itemCommand(itemKey);
    }

    qCWarning(logDock) << "Unknown item command requested for key:" << itemKey;
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
    qCDebug(logDock) << "Setting sort key for item:" << itemKey << "Key:" << key << "Order:" << order;
}

const QString DdeGrandSearchDockPlugin::itemContextMenu(const QString &itemKey)
{
    qCDebug(logDock) << "Generating context menu for item:" << itemKey;

    if (itemKey != GrandSearchPlugin && itemKey != QUICK_ITEM_KEY) {
        qCWarning(logDock) << "Context menu requested for unknown item:" << itemKey;
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
    qCDebug(logDock) << "Menu item invoked - Item:" << itemKey << "Menu:" << menuId << "Checked:" << checked;

    if (itemKey != GrandSearchPlugin && itemKey != QUICK_ITEM_KEY) {
        qCWarning(logDock) << "Menu invoked for unknown item:" << itemKey;
        return;
    }

    if (menuId == MenuOpenSetting) {
        QProcess::startDetached("dbus-send", QStringList() << "--print-reply" << "--dest=com.deepin.dde.GrandSearchSetting" << "/com/deepin/dde/GrandSearchSetting" << "com.deepin.dde.GrandSearchSetting.Show");
    }

    m_proxyInter->requestSetAppletVisible(this, pluginName(), false);
}

#if (QT_VERSION_MAJOR < 6)
void DdeGrandSearchDockPlugin::onGsettingsChanged(const QString &key)
{
    Q_ASSERT(m_gsettings);

    qCDebug(logDock) << "GSettings value changed - Key:" << key << "Value:" << m_gsettings->get(key);
    if (key == SchemaKeyMenuEnable) {
        bool enable = m_gsettings->get(key).toBool();
        qCInfo(logDock) << "Grand search context menu state changed - Enabled:" << enable;
    }
}
#endif
void DdeGrandSearchDockPlugin::onVisibleChanged(bool visible)
{
    qCDebug(logDock) << "Grand Search visibility changed:" << visible;

#ifdef USE_DOCK_2_0
    if (!m_messageCallback) {
        qCWarning(logDock) << "Message callback function not initialized";
        return;
    }

    QJsonObject obj;
    obj[Dock::MSG_TYPE] = Dock::MSG_ITEM_ACTIVE_STATE;
    obj[Dock::MSG_DATA] = visible;
    QJsonDocument doc;
    doc.setObject(obj);
    m_messageCallback(this, doc.toJson());
    qCDebug(logDock) << "Dock 2.0 message callback executed for visibility change";
#endif
}
