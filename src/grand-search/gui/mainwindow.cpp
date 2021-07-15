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
#include "mainwindow_p.h"
#include "mainwindow.h"
#include "entrance/entrancewidget.h"
#include "exhibition/exhibitionwidget.h"

#include <QDebug>
#include <QTimer>
#include <QVBoxLayout>

class MainWindowGlobal : public MainWindow {};
Q_GLOBAL_STATIC(MainWindowGlobal, mainWindowGlobal)

DWIDGET_USE_NAMESPACE

MainWindowPrivate::MainWindowPrivate(MainWindow *parent)
    : q_p(parent)
{

}

MainWindow::MainWindow(QWidget *parent)
    : DBlurEffectWidget(parent)
    , d_p(new MainWindowPrivate(this))
{
    initUI();
    initConnect();
}

MainWindow::~MainWindow()
{

}

void MainWindow::connectToController()
{
    d_p->m_entranceWidget->connectToController();
    d_p->m_exhibitionWidget->connectToController();
}

MainWindow *MainWindow::instance()
{
    return mainWindowGlobal;
}

void MainWindow::initUI()
{
    move(100, 100);
    //  设置窗口标识为无边框、不在任务栏显示
//    Qt::WindowFlags flags = windowFlags();
//     flags |= Qt::Tool | Qt::FramelessWindowHint;
//    setWindowFlags(flags);

//    // 模糊半径
//    setRadius(30);
//    // 透明背景
//    setAttribute(Qt::WA_TranslucentBackground);
//    // 焦点策略
//    setFocusPolicy(Qt::NoFocus);
//    // 模糊模式
    setBlendMode(DBlurEffectWidget::BehindWindowBlend);
//    // X和Y方向的圆角半径
//    setBlurRectXRadius(10);
//    setBlurRectYRadius(10);
//    // 遮罩层颜色设置
//    setMaskColor(DBlurEffectWidget::DarkColor);

    // 搜索入口界面
    d_p->m_entranceWidget = new EntranceWidget(this);

    // 结果展示界面
    d_p->m_exhibitionWidget = new ExhibitionWidget(this);

    d_p->m_mainLayout = new QVBoxLayout(this);
    d_p->m_mainLayout->setSpacing(0);
    d_p->m_mainLayout->addWidget(d_p->m_entranceWidget);
    d_p->m_mainLayout->addWidget(d_p->m_exhibitionWidget);
    this->setLayout(d_p->m_mainLayout);

    this->resize(760, 520);
}

void MainWindow::initConnect()
{
    connect(d_p->m_entranceWidget, &EntranceWidget::searchTextChanged, this, &MainWindow::searchTextChanged);
}

void MainWindow::showEvent(QShowEvent *event)
{
    emit visibleChanged(true);
    return DBlurEffectWidget::showEvent(event);
}

void MainWindow::hideEvent(QHideEvent *event)
{
    emit visibleChanged(false);
    return DBlurEffectWidget::hideEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // todo 通知controller停止处理，搜索任务的controller将通知后端停止搜索
    return DBlurEffectWidget::closeEvent(event);
}
