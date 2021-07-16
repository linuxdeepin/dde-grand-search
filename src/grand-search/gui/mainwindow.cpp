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
#include "gui/datadefine.h"

#include <DApplication>

#include <QDebug>
#include <QTimer>
#include <QVBoxLayout>
#include <QScreen>

class MainWindowGlobal : public MainWindow {};
Q_GLOBAL_STATIC(MainWindowGlobal, mainWindowGlobal)

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

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

void MainWindow::onPrimaryScreenChanged(QScreen *screen)
{
    // 主窗口显示在主屏
    disconnect(this, SLOT(onGeometryChanged(const QRect &)));
    onGeometryChanged(screen->geometry());
    connect(screen, &QScreen::geometryChanged, this, &MainWindow::onGeometryChanged);
}

void MainWindow::onGeometryChanged(const QRect &geometry)
{
    int sWidth = geometry.width();
    int sHeight = geometry.height();

    // 移动窗口到屏幕的居中偏上位置
    sWidth = geometry.x() + sWidth/2 - int(MainWindowWidth/2);
    sHeight = geometry.y() + sHeight/4;
    move(sWidth, sHeight);
}

MainWindow *MainWindow::instance()
{
    return mainWindowGlobal;
}

void MainWindow::initUI()
{
    //  设置窗口标识不在任务栏显示
    setWindowFlag(Qt::Tool, true);

    // 控制界面大小和位置
    setFixedSize(MainWindowWidth, MainWindowHeight);
    QScreen *screen = qApp->primaryScreen();
    onPrimaryScreenChanged(screen);

    // 模糊模式
    setBlendMode(DBlurEffectWidget::BehindWindowBlend);

    // 搜索入口界面
    d_p->m_entranceWidget = new EntranceWidget(this);

    // 结果展示界面
    d_p->m_exhibitionWidget = new ExhibitionWidget(this);
    d_p->m_exhibitionWidget->hide();

    d_p->m_mainLayout = new QVBoxLayout(this);
    d_p->m_mainLayout->addWidget(d_p->m_entranceWidget);
    d_p->m_mainLayout->addWidget(d_p->m_exhibitionWidget);

    // 根据设计图调整主界面布局，限制边距和内容间距为0
    d_p->m_mainLayout->setSpacing(0);
    d_p->m_mainLayout->setMargin(0);

    this->setLayout(d_p->m_mainLayout);
}

void MainWindow::initConnect()
{
    // 通知查询控制模块发起新的搜索
    connect(d_p->m_entranceWidget, &EntranceWidget::searchTextChanged, this, &MainWindow::searchTextChanged);

    // 展开展示界面
    connect(d_p->m_entranceWidget, &EntranceWidget::searchTextChanged, this, &MainWindow::showExhibitionWidget);

    // 监控主屏改变信号，及时更新窗口位置
    connect(qApp, &QGuiApplication::primaryScreenChanged, this, &MainWindow::onPrimaryScreenChanged);
}

void MainWindow::showExhibitionWidget(const QString &txt)
{
    if (txt.isEmpty() && !d_p->m_exhibitionWidget->isHidden()) {
        setFixedSize(MainWindowWidth, MainWindowHeight);
        d_p->m_exhibitionWidget->hide();
    } else if (!txt.isEmpty() && d_p->m_exhibitionWidget->isHidden()){
        setFixedSize(MainWindowWidth, MainWindowExpandHeight);
        d_p->m_exhibitionWidget->show();
    }
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
    // 通知查询控制器停止搜索
    emit terminateSearch();
    return DBlurEffectWidget::closeEvent(event);
}
