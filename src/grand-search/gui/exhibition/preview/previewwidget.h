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
#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include "previewpluginmanager.h"
#include <DWidget>

#include <QScopedPointer>

class PreviewWidgetPrivate;
class QVBoxLayout;
class PreviewWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
public:
    explicit PreviewWidget(QWidget *parent = nullptr);
    ~PreviewWidget() override;

    // 预览指定搜索项
    bool previewItem(const GrandSearch::MatchedItem &item);

private:
    void initUi();
    void initConnect();
    void reLayout();
    void clearLayoutWidgets();

    void paintEvent(QPaintEvent *event) override;

private:
    QScopedPointer<PreviewWidgetPrivate> d_p;

    QVBoxLayout *m_vMainLayout = nullptr;               // 预览界面主体布局

    PreviewPlugin* m_preview = nullptr;

    PreviewPluginManager m_pluginManager;
};

#endif // PREVIEWWIDGET_H
