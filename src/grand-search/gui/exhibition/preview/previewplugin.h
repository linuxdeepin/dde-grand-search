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

typedef QHash<QString, QString> DetailInfo;
typedef QList<DetailInfo> DetailInfoList;

class PreviewPlugin : public QObject
{
    Q_OBJECT
public:
    explicit PreviewPlugin(QObject *parent = nullptr);
    ~PreviewPlugin() override;

    // 预览指定的搜索结果项
    virtual bool previewItem(const GrandSearch::MatchedItem &item) = 0;

    // 获取当前预览的搜索项
    virtual GrandSearch::MatchedItem item() const = 0;

    // 停止预览
    virtual bool stopPreview() const;

    // 返回预览插件自定义预览内容部件
    virtual QWidget *contentWidget() const = 0;

    /**
     * @brief  获取属性详情信息，由预览主界面的属性详情部件显示具体信息
     * @state  若返回空，则不显示属性详部件
     * @return QWidget* 预览插件自定义工具栏
     */
    virtual DetailInfoList getAttributeDetailInfo() const;

    /**
     * @brief  返回预览插件自定义工具栏
     * @state  若返回空，且showToolBar返回true，则预览主界面显示默认工具栏(即显示打开、打开路径、复制路径按钮)
     * @return QWidget* 预览插件自定义工具栏
     */
    virtual QWidget *toolBarWidget() const; 

    /**
     * @brief  查询接口，供预览主界面调用
     * @return bool true: 预览主界面显示工具栏, false: 预览主界面不显示工具栏
     */
    virtual bool showToolBar() const;

    /**
     * @brief  返回预览界面有效点击区域(包括超出预览主界面外的菜单栏区域),
     *         搜索主界面通过该接口，能够正常管理主界面显隐逻辑
     * @return QRect 超出预览界面以外的可点击的区域
     */
    virtual QRect getValidClickRegion() const;
};

#endif // PREVIEWPLUGINUI_H
