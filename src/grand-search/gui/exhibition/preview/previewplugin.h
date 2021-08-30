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

#include "global/matcheditem.h"

#include <QObject>

//#define SHOW_PREVIEW_BACKCOLOR

class PreviewPlugin : public QObject
{
    Q_OBJECT
public:
    explicit PreviewPlugin(QObject *parent = 0);
    ~PreviewPlugin() override;

    // 预览指定的搜索结果项
    virtual bool previewItem(const GrandSearch::MatchedItem &item) = 0;

    // 获取当前预览的搜索项
    virtual GrandSearch::MatchedItem item() const = 0;

    // 返回预览插件自定义预览内容部件
    virtual QWidget *contentWidget() const = 0;

    // 停止预览
    virtual bool stopPreview() const;

    // 返回预览插件自定义状态栏
    virtual QWidget *statusBarWidget() const;

    // 返回预览插件自定义工具栏
    virtual QWidget *toolBarWidget() const;

    // 预览主界面是否显示状态栏
    virtual bool showStatusBar() const;

    // 预览主界面是否显示工具栏
    virtual bool showToolBar() const;
};

#endif // PREVIEWPLUGINUI_H
