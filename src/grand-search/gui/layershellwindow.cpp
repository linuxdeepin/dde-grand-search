// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "layershellwindow.h"

#include <QLoggingCategory>
#include <QScreen>
#include <QWidget>
#include <QWindow>

#ifdef HAVE_DDE_LAYER_SHELL
#include <dlayershellwindow.h>
#endif

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

namespace GrandSearch {

class LayerShellWindowPrivate
{
public:
    QWidget *widget = nullptr;
    bool valid = false;
#ifdef HAVE_DDE_LAYER_SHELL
    // 所有权不转移，DLayerShellWindow 生命周期由 dde-shell 内部管理。
    ds::DLayerShellWindow *shell = nullptr;
#endif
};

LayerShellWindow::LayerShellWindow(QWidget *widget)
    : d_p(new LayerShellWindowPrivate)
{
    d_p->widget = widget;

#ifdef HAVE_DDE_LAYER_SHELL
    if (!widget) {
        qCWarning(logGrandSearch) << "LayerShellWindow: invalid widget, layer shell disabled";
        return;
    }

    // DLayerShellWindow::get() 依赖 windowHandle() 已存在，先创建原生窗口句柄。
    widget->winId();
    QWindow *window = widget->windowHandle();
    if (!window) {
        qCWarning(logGrandSearch) << "LayerShellWindow: failed to obtain QWindow handle";
        return;
    }

    d_p->shell = ds::DLayerShellWindow::get(window);
    if (!d_p->shell) {
        qCWarning(logGrandSearch) << "LayerShellWindow: failed to obtain layer shell instance";
        return;
    }

    // 全局搜索弹层：贴顶且水平自动居中，置于 overlay 层，允许键盘交互。
    d_p->shell->setAnchors(ds::DLayerShellWindow::AnchorTop);
    d_p->shell->setLayer(ds::DLayerShellWindow::LayerOverlay);
    d_p->shell->setKeyboardInteractivity(ds::DLayerShellWindow::KeyboardInteractivityOnDemand);
    d_p->shell->setScope(QStringLiteral("dde-grand-search"));
    d_p->shell->setScreenConfiguration(ds::DLayerShellWindow::ScreenFromQWindow);
    d_p->shell->setCloseOnDismissed(false);

    d_p->valid = true;
#endif
}

LayerShellWindow::~LayerShellWindow()
{
    // d_p->shell 的所有权不由本类持有，无需释放。
}

bool LayerShellWindow::isValid() const
{
    return d_p->valid;
}

void LayerShellWindow::setTopAnchor(int topMargin)
{
#ifdef HAVE_DDE_LAYER_SHELL
    if (!d_p->valid || !d_p->shell)
        return;

    d_p->shell->setAnchors(ds::DLayerShellWindow::AnchorTop);
    d_p->shell->setTopMargin(topMargin);
#else
    Q_UNUSED(topMargin);
#endif
}

void LayerShellWindow::setPreferredSize(int width, int height)
{
#ifdef HAVE_DDE_LAYER_SHELL
    if (!d_p->valid || !d_p->shell)
        return;

    d_p->shell->setPreferredWidth(width);
    d_p->shell->setPreferredHeight(height);
#else
    Q_UNUSED(width);
    Q_UNUSED(height);
#endif
}

void LayerShellWindow::setScreen(QScreen *screen)
{
#ifdef HAVE_DDE_LAYER_SHELL
    if (!d_p->valid || !d_p->widget || !screen)
        return;

    // ScreenFromQWindow 配置下，layer surface 跟随 QWindow::screen()。
    if (QWindow *window = d_p->widget->windowHandle())
        window->setScreen(screen);
#else
    Q_UNUSED(screen);
#endif
}

}
