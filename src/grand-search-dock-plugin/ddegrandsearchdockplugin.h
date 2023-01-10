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
#ifndef DDEGRANDSEARCHDOCKPLUGIN_H
#define DDEGRANDSEARCHDOCKPLUGIN_H

#include <pluginsiteminterface.h>

#include <QObject>
#include <QScopedPointer>

class QGSettings;

namespace GrandSearch {

class TipsWidget;
class GrandSearchWidget;
class DdeGrandSearchDockPlugin : public QObject, PluginsItemInterface
{
    Q_OBJECT
    // 声明实现了的接口
    Q_INTERFACES(PluginsItemInterface)
    // 插件元数据
    Q_PLUGIN_METADATA(IID "com.deepin.dock.PluginsItemInterface" FILE "ddegrandsearch_dockplugin.json")

public:
    explicit DdeGrandSearchDockPlugin(QObject *parent = nullptr);
    ~DdeGrandSearchDockPlugin() override;

    // 返回插件的名称，必须是唯一值，不可以和其它插件冲突
    const QString pluginName() const override;

    // 返回插件展示名称
    const QString pluginDisplayName() const override;

    // 插件初始化函数
    void init(PluginProxyInterface *proxyInter) override;

    // 返回插件的 widget
    QWidget *itemWidget(const QString &itemKey) override;

    // 返回插件的 tipsWidget
    QWidget *itemTipsWidget(const QString &itemKey) override;

    // 返回插件是否允许被禁用
    bool pluginIsAllowDisable() override;

    // 返回插件当前是否处于被禁用状态
    bool pluginIsDisable() override;

    // 当插件的禁用状态被用户改变时此接口被调用
    void pluginStateSwitched() override;

    // 返回鼠标左键点击插件主控件后要执行的命令数据（鼠标释放时调用）
    const QString itemCommand(const QString &itemKey) override;

    // 返回插件主控件的排序位置
    int itemSortKey(const QString &itemKey) override;

    // 重新设置主控件新的排序位置（用户拖动了插件控件后）
    void setSortKey(const QString &itemKey, const int order) override;

    // 右键菜单
    const QString itemContextMenu(const QString &itemKey) override;

    // 菜单执行项
    void invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked) override;

    QIcon icon(const DockPart &dockPart, DGuiApplicationHelper::ColorType themeType = DGuiApplicationHelper::instance()->themeType()) override;

private slots:
    void onGsettingsChanged(const QString &key);

private:
    QScopedPointer<GrandSearchWidget> m_searchWidget;
    QScopedPointer<QGSettings> m_gsettings;
    QScopedPointer<TipsWidget> m_tipsWidget;
};

}

#endif   // DDEGRANDSEARCHDOCKPLUGIN_H
