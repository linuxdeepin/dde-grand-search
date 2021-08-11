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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DBlurEffectWidget>

#include <QScopedPointer>

class MainWindowPrivate;
class MainWindow : public Dtk::Widget::DBlurEffectWidget
{
    Q_OBJECT
public:
    static MainWindow *instance();
    ~MainWindow() Q_DECL_OVERRIDE;

    // 必须在主界面显示后再调用该函数，处理业务相关流程
    void connectToController();

    void showExhitionWidget(bool bShow = true);
    void showSerachNoContent(bool bShow = true);

    void showEntranceAppIcon(bool bShow = true);

    // 获取有效区域，包含主界面和菜单区域
    QRegion getValidRegion();

private slots:
    // 主屏改变响应槽，移动已显示的主界面到主屏
    void onPrimaryScreenChanged(const QScreen *screen);
    // 主屏分辨率改变响应槽，调整显示的位置
    void onGeometryChanged(const QRect &geometry);
    // 根据是否发起搜索，调整展示界面是否显示
    void onSearchTextChanged(const QString &txt);

private:
    void initUI();
    void initConnect();

    void activeMainWindow();
    void updateMainWindowHeight();

protected:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    virtual void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    virtual void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    virtual void hideEvent(QHideEvent *event) Q_DECL_OVERRIDE;
    virtual void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;

signals:
    // 显示状态改变信号
    void visibleChanged(const bool visible);
    // 搜索文本改变信号，通知查询控制器发起新的搜索
    void searchTextChanged(const QString &txt);
    // 终止搜索信号，软件退出前通知后端终止搜索
    void terminateSearch();

private:
    QScopedPointer<MainWindowPrivate> d_p;
};

#endif // MAINWINDOW_H
