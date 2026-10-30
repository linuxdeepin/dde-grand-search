// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <DBlurEffectWidget>

#include <QScopedPointer>

namespace GrandSearch {

class MainWindowPrivate;
class MainWindow : public Dtk::Widget::DBlurEffectWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() Q_DECL_OVERRIDE;

    // 必须在主界面显示后再调用该函数，处理业务相关流程
    void connectToController();
    // 主搜索框刚显示（500ms 内）时为宽限期，失焦不立即关闭——
    // 保存/打开文件对话框等模态场景下抢焦点存在窗口期，避免一显示就被判定失焦消失
    bool isGracePeriodActive() const;
    // 本次显示后是否从未获得过激活（模态窗口把焦点锁死时的典型状态）；
    // 该状态下失焦不再关闭主窗，保证呼出后保持显示
    bool neverActivatedSinceShow() const;
    // 记录主窗已获得激活（HandleVisibility 收到 ApplicationActive 时调用）
    void markActivated();

private slots:
    // 主屏改变响应槽，移动已显示的主界面到主屏
    void onPrimaryScreenChanged(const QScreen *screen);
    // 主屏分辨率改变响应槽，调整显示的位置
    void onGeometryChanged(const QRect &geometry);
    // 隐藏展示界面
    void onHideExhitionWidget();
    // 重置展示界面
    void onResetExhitionWidget(const QString &missionId);
    // 搜索任务改变
    void onMissionChanged(const QString &missionId, const QString &missionContent);

private:
    void initUI();
    void initConnect();

    void activeMainWindow();
    void updateMainWindowHeight();

    void showExhitionWidget(bool bShow = true);
    void showSerachNoContent(bool bShow = true);
    void showEntranceAppIcon(bool bShow = true);

    void onFocusObjectChanged(QObject *obj);

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
    qint64 m_lastShowTime = 0;        // 最近一次 show 的时间戳（宽限期判断用）
    bool m_everActivatedSinceShow = false;   // 本次显示后是否曾获得激活
    QScopedPointer<MainWindowPrivate> d_p;
};

}

#endif // MAINWINDOW_H
