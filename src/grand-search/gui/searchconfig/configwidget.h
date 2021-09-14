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
#ifndef CONFIGWIDGET_H
#define CONFIGWIDGET_H

#include <DMainWindow>
#include <DWidget>

#include <QVBoxLayout>

class ScopeWidget;
class PlanWidget;

class ConfigWidget : public Dtk::Widget::DMainWindow
{
    Q_OBJECT
public:
    explicit ConfigWidget(QWidget *parent = nullptr);
    ~ConfigWidget();

private:
    void initUI();
    void initData();

private:
    QVBoxLayout *m_mainLayout                       = nullptr;      // 界面主体布局
    Dtk::Widget::DScrollArea *m_scrollArea          = nullptr;      // 滚动区域部件
    Dtk::Widget::DWidget *m_scrollAreaContent       = nullptr;      // 滚动区域内容部件
    QVBoxLayout *m_scrollLayout                     = nullptr;      // 滚动区域内部部件布局

    ScopeWidget *m_searchGroupWidget          = nullptr;
    PlanWidget *m_searchPlanWidget            = nullptr;
};

#endif // CONFIGWIDGET_H