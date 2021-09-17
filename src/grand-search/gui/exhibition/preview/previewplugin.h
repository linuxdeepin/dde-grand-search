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
#ifndef PREVIEWPLUGINUI_H
#define PREVIEWPLUGINUI_H

#include "previewproxyinterface.h"

#include <QObject>
#include <QHash>
#include <QPair>

namespace GrandSearch {

typedef QPair<QString, QString> DetailInfo;
typedef QList<DetailInfo> DetailInfoList;

// 预览项目信息
// ItemInfo key
#define PREVIEW_ITEMINFO_ITEM "item"
#define PREVIEW_ITEMINFO_NAME "name"
#define PREVIEW_ITEMINFO_ICON "icon"
#define PREVIEW_ITEMINFO_TYPE "type"
#define PREVIEW_ITEMINFO_SEARCHER "searcher"
typedef QHash<QString, QString> ItemInfo;

class PreviewPlugin
{
public:
    virtual ~PreviewPlugin(){}

    // 插件初始化函数
    virtual void init(QObject *proxyInter) = 0;

    // 预览指定的搜索结果项
    virtual bool previewItem(const ItemInfo &item) = 0;

    // 获取当前预览的搜索项
    virtual ItemInfo item() const = 0;

    // 停止预览
    virtual bool stopPreview() = 0;

    // 返回预览插件自定义预览内容部件
    virtual QWidget *contentWidget() const = 0;

    /**
     * @brief  获取属性详情信息，由预览主界面的属性详情部件显示具体信息
     * @state  若返回空，则不显示属性详部件
     * @return QWidget* 预览插件自定义工具栏
     */
    virtual DetailInfoList getAttributeDetailInfo() const = 0;

    /**
     * @brief  返回预览插件自定义工具栏
     * @state  若返回空，且showToolBar返回true，则预览主界面显示默认工具栏(即显示打开、打开路径、复制路径按钮)
     * @return QWidget* 预览插件自定义工具栏
     */
    virtual QWidget *toolBarWidget() const = 0;

    /**
     * @brief  查询接口，供预览主界面调用
     * @return bool true: 预览主界面显示工具栏, false: 预览主界面不显示工具栏
     */
    virtual bool showToolBar() const = 0;
};

}
#endif // PREVIEWPLUGINUI_H
