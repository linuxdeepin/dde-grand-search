// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LAYERSHELLWINDOW_H
#define LAYERSHELLWINDOW_H

#include <QScopedPointer>

class QWidget;
class QScreen;

namespace GrandSearch {

class LayerShellWindowPrivate;

/*!
 * \brief The LayerShellWindow class
 *
 * 对 dde-shell 提供的 LayerShell 抽象（ds::DLayerShellWindow）做薄封装，
 * 用于在 Treeland（Wayland）下控制全局搜索窗口的层级、锚点与尺寸。
 *
 * 仅在编译期定义了 HAVE_DDE_LAYER_SHELL 时才会真正创建 layer surface；
 * 其余环境（X11、Qt5、未安装 dde-shell）下退化为空实现，isValid() 恒为 false。
 */
class LayerShellWindow
{
public:
    explicit LayerShellWindow(QWidget *widget);
    ~LayerShellWindow();

    LayerShellWindow(const LayerShellWindow &) = delete;
    LayerShellWindow &operator=(const LayerShellWindow &) = delete;

    /*!
     * \brief isValid 返回 layer shell 是否成功初始化。
     */
    bool isValid() const;

    /*!
     * \brief setTopAnchor 将窗口锚定在屏幕顶部（水平自动居中），并设置距顶边距。
     */
    void setTopAnchor(int topMargin);

    /*!
     * \brief setPreferredSize 设置 layer surface 的首选尺寸。
     */
    void setPreferredSize(int width, int height);

    /*!
     * \brief setScreen 设置 layer surface 跟随的屏幕（等价于设置 QWindow::screen()）。
     */
    void setScreen(QScreen *screen);

private:
    QScopedPointer<LayerShellWindowPrivate> d_p;
};

}

#endif // LAYERSHELLWINDOW_H
