// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow_p.h"
#include "mainwindow.h"
#include "layershellwindow.h"
#include "utils/utils.h"
#include "entrance/entrancewidget.h"
#include "exhibition/exhibitionwidget.h"
#include "gui/datadefine.h"
#include "gui/handlevisibility/handlevisibility.h"
#include "business/query/querycontroller.h"
#include "business/matchresult/matchcontroller.h"

#include <DApplication>
#include <DLabel>
#include <DRegionMonitor>
#include <DPlatformHandle>

#include <QDebug>
#include <QTimer>
#include <QVBoxLayout>
#include <QScreen>
#include <QKeyEvent>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

static const uint MainWindowWidth           = 760;      // 主界面宽度
static const uint MainWindowHeight          = 68;       // 主界面高度
static const uint MainWindowExpandHeight    = 520;      // 主界面高度

MainWindowPrivate::MainWindowPrivate(MainWindow *parent)
    : q_p(parent)
{
    qCDebug(logGrandSearch) << "Initializing MainWindow private data";
    m_handleVisibility = new HandleVisibility(q_p, q_p);
}

MainWindow::MainWindow(QWidget *parent)
    : DBlurEffectWidget(parent)
    , d_p(new MainWindowPrivate(this))
{
    qCDebug(logGrandSearch) << "Creating MainWindow";
    initUI();
    initConnect();
    qCInfo(logGrandSearch) << "MainWindow created successfully";
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    activeMainWindow();
    return DBlurEffectWidget::mousePressEvent(event);
}

MainWindow::~MainWindow()
{
    qCDebug(logGrandSearch) << "Destroying MainWindow";
}

void MainWindow::connectToController()
{
    Q_ASSERT(d_p->m_entranceWidget);
    Q_ASSERT(d_p->m_exhibitionWidget);
    Q_ASSERT(!d_p->m_queryController);
    Q_ASSERT(!d_p->m_matchController);

    qCDebug(logGrandSearch) << "Connecting to controllers";

    d_p->m_queryController = new QueryController(this);
    d_p->m_matchController = new MatchController(this);

    // 通知查询控制模块发起新的搜索
    connect(d_p->m_entranceWidget, &EntranceWidget::searchTextChanged, d_p->m_queryController, &QueryController::onSearchTextChanged);

    // 通知查询控制器停止搜索
    connect(this, &MainWindow::terminateSearch, d_p->m_queryController, &QueryController::onTerminateSearch);

    // 同步查询搜索任务
    connect(d_p->m_queryController, &QueryController::missionChanged, d_p->m_matchController, &MatchController::onMissionChanged);
    connect(d_p->m_queryController, &QueryController::missionChanged, this, &MainWindow::onMissionChanged);

    // 匹配结果解析显示
    connect(d_p->m_matchController, &MatchController::matchedResult, d_p->m_exhibitionWidget, &ExhibitionWidget::appendMatchedData);
    connect(d_p->m_matchController, &MatchController::searchCompleted, d_p->m_exhibitionWidget, &ExhibitionWidget::onSearchCompleted);

    qCInfo(logGrandSearch) << "Controller connections established";
}

void MainWindow::showExhitionWidget(bool bShow)
{
    Q_ASSERT(d_p->m_exhibitionWidget);

    //已经显示/隐藏，就不再重复显示/隐藏
    if (d_p->m_exhibitionWidget && !d_p->m_exhibitionWidget->isHidden() == bShow)
        return;

    d_p->m_exhibitionWidget->setVisible(bShow);
}

void MainWindow::showSerachNoContent(bool bShow)
{
    Q_ASSERT(d_p->m_entranceWidget);
    Q_ASSERT(d_p->m_exhibitionWidget);

    if (Q_LIKELY(d_p->m_searchNoContentWidget))
        d_p->m_searchNoContentWidget->setVisible(bShow);

    if (bShow) {
        if (Q_LIKELY(d_p->m_exhibitionWidget))
            d_p->m_exhibitionWidget->setVisible(false);
        // 显示无搜索结果时，必定在搜索入口界面中不显示应用图标
        showEntranceAppIcon(false);
    }
}

void MainWindow::showEntranceAppIcon(bool bShow)
{
    Q_ASSERT(d_p->m_entranceWidget);

    d_p->m_entranceWidget->showLabelAppIcon(bShow);
}

void MainWindow::onFocusObjectChanged(QObject *obj)
{
    // 焦点回到主窗口时，将焦点设置到输入框中(焦点改变到了预览相关的窗口中则不做处理)
    if (obj && obj == this && d_p->m_entranceWidget) {
        qCDebug(logGrandSearch) << "Focus returned to main window - Setting focus to entrance widget";
        d_p->m_entranceWidget->setFocus();
    }
}

void MainWindow::onPrimaryScreenChanged(const QScreen *screen)
{
    // 主窗口显示在主屏
    qCDebug(logGrandSearch) << "Primary screen changed - Geometry:" << screen->geometry()
                            << "DPI:" << screen->logicalDotsPerInch();
    disconnect(this, SLOT(onGeometryChanged(const QRect &)));
    onGeometryChanged(screen->geometry());
    connect(screen, &QScreen::geometryChanged, this, &MainWindow::onGeometryChanged);

    // Wayland 下 move() 无效，layer surface 需通过 QWindow::setScreen 跟随主屏。
    if (d_p->m_layerShellWindow && d_p->m_layerShellWindow->isValid()) {
        d_p->m_layerShellWindow->setScreen(const_cast<QScreen *>(screen));
    }
}

void MainWindow::onGeometryChanged(const QRect &geometry)
{
    // LayerShell 可用时，通过顶部锚点 + 距顶边距控制窗口位置（水平方向自动居中）。
    if (d_p->m_layerShellWindow && d_p->m_layerShellWindow->isValid()) {
        int topMargin = geometry.height() / 4;
        qCDebug(logGrandSearch) << "Window geometry changed - Screen:" << geometry
                                << "Setting layer shell top margin:" << topMargin;
        d_p->m_layerShellWindow->setTopAnchor(topMargin);
        return;
    }

    int sWidth = geometry.width();
    int sHeight = geometry.height();

    // 移动窗口到屏幕的居中偏上位置
    sWidth = geometry.x() + sWidth/2 - int(MainWindowWidth/2);
    sHeight = geometry.y() + sHeight/4;
    qCDebug(logGrandSearch) << "Window geometry changed - Screen:" << geometry
                            << "Moving window to:" << QPoint(sWidth, sHeight);
    move(sWidth, sHeight);
}

void MainWindow::onHideExhitionWidget()
{
    // 搜索文本为空，隐藏展示界面
    qCDebug(logGrandSearch) << "Hiding exhibition widget - Clearing search results";
    showSerachNoContent(false);
    showExhitionWidget(false);
    showEntranceAppIcon(false);

    updateMainWindowHeight();

    Q_ASSERT(d_p->m_exhibitionWidget);
    d_p->m_exhibitionWidget->clearData();
}

void MainWindow::onResetExhitionWidget(const QString &missionId)
{
    Q_UNUSED(missionId);

    // 搜索任务id改变，重置展示界面
    qCDebug(logGrandSearch) << "Resetting exhibition widget - Mission ID:" << missionId;
    showSerachNoContent(false);
    showExhitionWidget(true);

    updateMainWindowHeight();

    Q_ASSERT(d_p->m_exhibitionWidget);
    d_p->m_exhibitionWidget->clearData();
}

void MainWindow::onMissionChanged(const QString &missionId, const QString &missionContent)
{
    qCDebug(logGrandSearch) << "Mission changed - ID:" << missionId
                            << "Content length:" << missionContent.length();
    onResetExhitionWidget(missionId);

    if (missionContent.isEmpty()) {
        onHideExhitionWidget();
    } else {
        // 搜索关键词改变，传播到 ExhibitionWidget 以管理高亮任务
        d_p->m_exhibitionWidget->setSearchKeyword(missionContent);
    }
}

void MainWindow::initUI()
{
    // 层级与窗口标志：
    // - LayerShell 可用（编译期支持 dde-shell 且运行于 Wayland）：层级交由 LayerShell 管理，
    //   不能设置 BypassWindowManagerHint（该标志会绕过合成器管理，与 layer surface 冲突）。
    // - Wayland 旧路径（未编译 dde-shell）：保持原有 BypassWindowManagerHint。
    // - X11：保持原有逻辑不变。
    if (Utils::isWayland()) {
#ifdef HAVE_DDE_LAYER_SHELL
        setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Tool);
#else
        setWindowFlags(Qt::BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::Tool);
#endif
        setAttribute(Qt::WA_QuitOnClose);
    } else {
        // 在Qt5.11版本、x11环境下，同时设置BypassWindowManagerHint与Tool属性之后，键盘切换大小写将会导致程序的激活状态发生改变。
        setWindowFlags(Qt::BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
    }
    qCInfo(logGrandSearch) << "Initializing main window - Platform:" << QApplication::platformName()
                          << "Window flags:" << this->windowFlags();

    // 控制界面大小和位置
    setFixedSize(MainWindowWidth, MainWindowHeight);
    QScreen *screen = qApp->primaryScreen();
    onPrimaryScreenChanged(screen);

    // 模糊模式
    setBlendMode(DBlurEffectWidget::BehindWindowBlend);

    // 搜索入口界面
    d_p->m_entranceWidget = new EntranceWidget(this);
    d_p->m_entranceWidget->setFixedHeight(MainWindowHeight);

    // 结果展示界面
    d_p->m_exhibitionWidget = new ExhibitionWidget(this);
    d_p->m_exhibitionWidget->hide();

    // 未搜到内容界面
    d_p->m_searchNoContentWidget = new DLabel(tr("No search results"),this);
    d_p->m_searchNoContentWidget->setAlignment(Qt::AlignCenter);
    d_p->m_searchNoContentWidget->hide();

    d_p->m_mainLayout = new QVBoxLayout(this);
    d_p->m_mainLayout->addWidget(d_p->m_entranceWidget);
    d_p->m_mainLayout->addWidget(d_p->m_exhibitionWidget);
    d_p->m_mainLayout->addWidget(d_p->m_searchNoContentWidget);

    // 根据设计图调整主界面布局，限制边距和内容间距为0
    d_p->m_mainLayout->setSpacing(0);
    d_p->m_mainLayout->setContentsMargins(0, 0, 0, 0);

    this->setLayout(d_p->m_mainLayout);

    this->winId();

    // 仅在 Wayland 下创建 LayerShell 适配；X11 下保持 m_layerShellWindow 为空，
    // 继续走原有 move()/setWindowFlags 逻辑。
    if (Utils::isWayland()) {
        d_p->m_layerShellWindow.reset(new LayerShellWindow(this));
        if (d_p->m_layerShellWindow->isValid()) {
            // 初始定位与初始尺寸，与 X11 路径的 setFixedSize + move 保持一致。
            d_p->m_layerShellWindow->setPreferredSize(MainWindowWidth, MainWindowHeight);
            if (QScreen *screen = qApp->primaryScreen()) {
                d_p->m_layerShellWindow->setTopAnchor(screen->geometry().height() / 4);
            }
        }
    }

    DPlatformHandle handle(this->windowHandle());
    handle.setWindowRadius(18);
}

void MainWindow::initConnect()
{
    Q_ASSERT(d_p->m_entranceWidget);
    Q_ASSERT(d_p->m_exhibitionWidget);

    // 监控主屏改变信号，及时更新窗口位置
    connect(qApp, &QGuiApplication::primaryScreenChanged, this, &MainWindow::onPrimaryScreenChanged);

    connect(d_p->m_entranceWidget, &EntranceWidget::sigSelectPreviousItem, d_p->m_exhibitionWidget, &ExhibitionWidget::onSelectPreviousItem);
    connect(d_p->m_entranceWidget, &EntranceWidget::sigSelectNextItem, d_p->m_exhibitionWidget, &ExhibitionWidget::onSelectNextItem);
    connect(d_p->m_entranceWidget, &EntranceWidget::sigHandleItem, d_p->m_exhibitionWidget, &ExhibitionWidget::onHandleItem);
    connect(d_p->m_entranceWidget, &EntranceWidget::sigCloseWindow, d_p->m_handleVisibility, &HandleVisibility::onCloseWindow);

    connect(d_p->m_exhibitionWidget, &ExhibitionWidget::sigCurrentItemChanged, d_p->m_entranceWidget, &EntranceWidget::onAppIconChanged);
    connect(d_p->m_exhibitionWidget, &ExhibitionWidget::sigShowNoContent, this, &MainWindow::showSerachNoContent);
    connect(d_p->m_exhibitionWidget, &ExhibitionWidget::sigCloseWindow, d_p->m_handleVisibility, &HandleVisibility::onCloseWindow);

    connect(qApp, &QGuiApplication::focusObjectChanged, this, &MainWindow::onFocusObjectChanged);
}

void MainWindow::activeMainWindow()
{
    activateWindow();
    raise();
}

void MainWindow::updateMainWindowHeight()
{
    Q_ASSERT(d_p->m_exhibitionWidget);
    Q_ASSERT(d_p->m_searchNoContentWidget);

    bool bExhibitionWidgetShow = false;
    bool bSearchNoContentWidgetShow = false;

    if (Q_LIKELY(d_p->m_exhibitionWidget))
        bExhibitionWidgetShow = !d_p->m_exhibitionWidget->isHidden();
    if (Q_LIKELY(d_p->m_searchNoContentWidget))
        bSearchNoContentWidgetShow = !d_p->m_searchNoContentWidget->isHidden();

    const uint targetHeight = (bExhibitionWidgetShow || bSearchNoContentWidgetShow)
            ? MainWindowExpandHeight
            : MainWindowHeight;
    this->setFixedSize(MainWindowWidth, targetHeight);

    // LayerShell 下 setFixedSize 不会生效为 layer surface 的尺寸，需同步首选尺寸。
    if (d_p->m_layerShellWindow && d_p->m_layerShellWindow->isValid())
        d_p->m_layerShellWindow->setPreferredSize(MainWindowWidth, targetHeight);
}

void MainWindow::showEvent(QShowEvent *event)
{
    qCDebug(logGrandSearch) << "MainWindow show event";
    QMetaObject::invokeMethod(this, "visibleChanged", Qt::QueuedConnection, Q_ARG(bool, true));

    // 已禁用窗口管理器，在窗口被显示后，需要激活该窗口
    QTimer::singleShot(1, this, &MainWindow::activeMainWindow);

    d_p->m_handleVisibility->registerRegion(true);

    return DBlurEffectWidget::showEvent(event);
}

void MainWindow::hideEvent(QHideEvent *event)
{
    qCDebug(logGrandSearch) << "MainWindow hide event";
    emit visibleChanged(false);

    d_p->m_handleVisibility->registerRegion(false);

    return DBlurEffectWidget::hideEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qCDebug(logGrandSearch) << "MainWindow close event";
    emit visibleChanged(false);
    // 通知查询控制器停止搜索
    emit terminateSearch();
    // FIXME: DBlurEffectWidget close abort on treeland
    QTimer::singleShot(1, this, [](){
        qApp->exit(0);
        _Exit(0);
    });
    // return DBlurEffectWidget::closeEvent(event);
}
