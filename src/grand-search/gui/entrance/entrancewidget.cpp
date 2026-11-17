// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "entrancewidget_p.h"
#include "entrancewidget.h"
#include "gui/datadefine.h"
#include "utils/utils.h"

#include <DSearchEdit>
#include <DStyle>
#include <DLabel>
#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DIconButton>

#include <QLineEdit>
#include <QPainter>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLayout>
#include <QToolButton>
#include <QWidgetAction>
#include <QAction>
#include <QTimer>
#include <QProxyStyle>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QMenu>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QEvent>
#include <QtGlobal>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(logGrandSearch)

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

static const uint DelayReponseTime = 50;   // 输入延迟搜索时间
static const uint EntraceWidgetWidth = 740;   // 搜索界面宽度度
static const uint EntraceWidgetHeight = 48;   // 搜索界面高度
static const uint WidgetMargins = 10;   // 界面边距
static const uint SearchMaxLength = 512;   // 输入最大字符限制

static const uint LabelIconSize = 26;   // 标签应用图标显示大小
static const uint LabelSize = 32;   // 标签大小
static const uint ClearIconSize = 30;   // 清除按钮图标绘制尺寸，与原生清除按钮一致
static const uint ClearIconArtworkInset = 5;   // 图标素材在绘制区域内的透明边
static const uint IconGap = 10;   // 清除按钮与应用图标之间的视觉间距
static const uint SearchIconSize = 28;   // 搜索框放大镜图标尺寸（DTK 默认 20，偏小导致视觉差异）

static const QString kSearchIconPath = QStringLiteral(":/icons/search_icon.svg");

// 放大 QLineEdit leading 搜索图标的绘制尺寸（QLineEditIconButton 图标大小由 PM_LineEditIconSize 决定，
// 与 action 图标尺寸无关，默认 20 偏小）
class LineEditIconStyle : public QProxyStyle
{
public:
    using QProxyStyle::QProxyStyle;

    int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr,
                    const QWidget *widget = nullptr) const override
    {
        if (metric == QStyle::PM_LineEditIconSize)
            return SearchIconSize;
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
};

// Qt5 原生光标使用 RasterOp_NotDestination 绘制，颜色恒为背景反色，无法满足设计要求的
// 固定光标颜色。此处将原生光标宽度置 0 隐藏，改由 CursorWidget 自绘光标。
class NoCursorStyle : public QProxyStyle
{
public:
    using QProxyStyle::QProxyStyle;

    int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr,
                    const QWidget *widget = nullptr) const override
    {
        if (metric == QStyle::PM_TextCursorWidth)
            return 0;
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
};

// 与 DTK 内置清除按钮外观一致的图标按钮：只绘制图标本身，不绘制按钮边框。
// 内置清除按钮的位置由 QLineEdit 的样式度量决定，会在应用图标左侧留下固定空档，
// 无法把两者间距精确调整到设计要求的 10px；改由本控件放入右侧容器统一控制间距。
class ClearIconButton : public QToolButton
{
public:
    explicit ClearIconButton(QWidget *parent = nullptr)
        : QToolButton(parent)
    {
        setCursor(Qt::ArrowCursor);
        setFocusPolicy(Qt::NoFocus);
        setToolButtonStyle(Qt::ToolButtonIconOnly);
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        const QIcon::Mode mode = isEnabled() ? QIcon::Normal : QIcon::Disabled;

        QRect iconRect(QPoint(0, 0), iconSize());
        iconRect.moveCenter(rect().center());
        icon().paint(&painter, iconRect, Qt::AlignCenter, mode);
    }
};

EntranceWidgetPrivate::EntranceWidgetPrivate(EntranceWidget *parent)
    : q_p(parent)
{
    m_delayChangeTimer = new QTimer(this);
    m_delayChangeTimer->setSingleShot(true);
    m_delayChangeTimer->setInterval(DelayReponseTime);

    connect(m_delayChangeTimer, &QTimer::timeout, this, &EntranceWidgetPrivate::notifyTextChanged);
}


void EntranceWidgetPrivate::updateTextMargin()
{
    if (!m_lineEdit || !m_clearButton)
        return;

    // 按右侧容器当前实际占用的宽度预留文本区域（隐藏的控件不计入 sizeHint）
    const QWidget *rightWidget = m_clearButton->parentWidget();
    const QLayout *layout = rightWidget ? rightWidget->layout() : nullptr;
    const int reserve = layout ? layout->sizeHint().width() : 0;

    const QMargins margins = m_lineEdit->textMargins();
    if (margins.right() != reserve)
        m_lineEdit->setTextMargins(margins.left(), margins.top(), reserve, margins.bottom());
}

void EntranceWidgetPrivate::delayChangeText()
{
    Q_ASSERT(m_delayChangeTimer);

    qCDebug(logGrandSearch) << "Search text input changed - Starting delay timer";
    m_delayChangeTimer->start();
}

void EntranceWidgetPrivate::notifyTextChanged()
{
    Q_ASSERT(m_searchEdit);

    const QString &currentSearchText = m_searchEdit->text().trimmed();
    qCDebug(logGrandSearch) << "Search text changed - Text:" << currentSearchText
                            << "Length:" << currentSearchText.length();
    emit q_p->searchTextChanged(currentSearchText);

    // 搜索内容改变后，仅在文本为空时清空图标显示
    // 文本非空时由搜索结果回调（sigCurrentItemChanged → onAppIconChanged）自然更新
    if (currentSearchText.isEmpty()) {
        MatchedItem item;
        q_p->onAppIconChanged(QString(), item);
    }
}

void EntranceWidgetPrivate::showMenu(const QPoint &pos)
{
    Q_ASSERT(m_lineEdit);

    qCDebug(logGrandSearch) << "Showing context menu - Position:" << pos;
    QMenu *menu = new QMenu;
    QAction *action = nullptr;

    action = menu->addAction(tr("Cut"));
    action->setEnabled(m_lineEdit->hasSelectedText() && m_lineEdit->echoMode() == QLineEdit::Normal);
    connect(action, &QAction::triggered, m_lineEdit, &QLineEdit::cut);

    action = menu->addAction(tr("Copy"));
    action->setEnabled(m_lineEdit->hasSelectedText() && m_lineEdit->echoMode() == QLineEdit::Normal);
    connect(action, &QAction::triggered, m_lineEdit, &QLineEdit::copy);

    action = menu->addAction(tr("Paste"));
    action->setEnabled(!QGuiApplication::clipboard()->text().isEmpty());
    connect(action, &QAction::triggered, m_lineEdit, &QLineEdit::paste);

    menu->exec(pos);
    delete menu;
}

EntranceWidget::EntranceWidget(QWidget *parent)
    : QFrame(parent), d_p(new EntranceWidgetPrivate(this))
{
    qCDebug(logGrandSearch) << "Creating EntranceWidget";
    initUI();
    initConnections();
    qCDebug(logGrandSearch) << "EntranceWidget created successfully";
}

EntranceWidget::~EntranceWidget()
{
    qCDebug(logGrandSearch) << "Destroying EntranceWidget";
}

void EntranceWidget::showLabelAppIcon(bool visible)
{
    Q_ASSERT(d_p->m_lineEdit);
    Q_ASSERT(d_p->m_appIconLabel);
    if (visible == d_p->m_appIconLabel->isVisible())
        return;

    qCDebug(logGrandSearch) << "App icon visibility changed - Visible:" << visible;
    d_p->m_appIconLabel->setVisible(visible);
    d_p->updateTextMargin();
    d_p->m_lineEdit->update();
}

bool EntranceWidget::event(QEvent *event)
{
    if (event->type() == QEvent::FocusIn) {
        qCDebug(logGrandSearch) << "Focus in event received";
        d_p->m_lineEdit->setFocus();
        return true;
    }

    return QFrame::event(event);
}

void EntranceWidget::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);
}

bool EntranceWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d_p->m_lineEdit && QEvent::KeyPress == event->type()) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (Q_LIKELY(keyEvent)) {
            int key = keyEvent->key();
            qCDebug(logGrandSearch) << "Key press event:" << key;
            switch (key) {
            case Qt::Key_Up: {
                qCDebug(logGrandSearch) << "Navigation key pressed - Selecting previous item";
                emit sigSelectPreviousItem();
                return true;
            }
            case Qt::Key_Tab:
            case Qt::Key_Down: {
                qCDebug(logGrandSearch) << "Navigation key pressed - Selecting next item";
                emit sigSelectNextItem();
                return true;
            }
            case Qt::Key_Return:
            case Qt::Key_Enter: {
                qCDebug(logGrandSearch) << "Enter key pressed - Handling selected item";
                emit sigHandleItem();
                return true;
            }
            case Qt::Key_Escape: {
                qCDebug(logGrandSearch) << "Escape key pressed - Closing window";
                emit sigCloseWindow();
                return true;
            }
            default:
                break;
            }
        }
    } else if (watched == d_p->m_lineEdit && QEvent::ContextMenu == event->type()) {
        QContextMenuEvent *contextMenuEvent = static_cast<QContextMenuEvent *>(event);
        if (Q_LIKELY(contextMenuEvent)) {
            qCDebug(logGrandSearch) << "Context menu event received";
            d_p->showMenu(contextMenuEvent->globalPos());
            return true;
        }
    } else if (watched == d_p->m_searchEdit && QEvent::FocusIn == event->type()) {
        if (Q_LIKELY(d_p->m_lineEdit)) {
            qCDebug(logGrandSearch) << "Search edit focus in event";
            d_p->m_lineEdit->setFocus();
            return true;
        }
    } else if (watched == d_p->m_lineEdit && QEvent::FocusIn == event->type()) {
        d_p->m_cursorOn = true;
        if (d_p->m_cursorBlinkTimer)
            d_p->m_cursorBlinkTimer->start();
        // 先隐藏光标，延迟到下一事件循环再定位，确保布局和文本变化（如清空操作）
        // 已完成，避免 ImCursorRectangle 返回未就绪的几何信息导致光标先闪在错误位置
        if (d_p->m_cursor)
            d_p->m_cursor->hide();
        QTimer::singleShot(0, this, &EntranceWidget::updateCursor);
    } else if (watched == d_p->m_lineEdit && QEvent::FocusOut == event->type()) {
        if (d_p->m_cursorBlinkTimer)
            d_p->m_cursorBlinkTimer->stop();
        if (d_p->m_cursor)
            d_p->m_cursor->hide();
    } else if (watched == d_p->m_lineEdit && QEvent::Resize == event->type()) {
        updateCursor();
    }
    return QFrame::eventFilter(watched, event);
}

void EntranceWidget::initUI()
{
    d_p->m_searchEdit = new DSearchEdit(this);
    d_p->m_searchEdit->installEventFilter(this);

    d_p->m_lineEdit = d_p->m_searchEdit->lineEdit();
    d_p->m_lineEdit->setMaxLength(SearchMaxLength);
    d_p->m_lineEdit->installEventFilter(this);

    QFont lineFont = d_p->m_lineEdit->font();
    lineFont = DFontSizeManager::instance()->get(DFontSizeManager::T4, lineFont);
    d_p->m_lineEdit->setFont(lineFont);

    updateLineEditPalette();

    // 隐藏 Qt 原生光标，改用自绘光标（原生光标为背景反色，无法满足设计的固定颜色要求）
    // NoCursorStyle 作为 LineEditIconStyle 的 base style 链式合并，避免 setStyle 覆盖
    auto *noCursorStyle = new NoCursorStyle();
    d_p->m_cursor = new CursorWidget(d_p->m_lineEdit);
    d_p->m_cursor->hide();

    d_p->m_cursorBlinkTimer = new QTimer(this);
    d_p->m_cursorBlinkTimer->setInterval(500);
    connect(d_p->m_cursorBlinkTimer, &QTimer::timeout, this, [this]() {
        d_p->m_cursorOn = !d_p->m_cursorOn;
        updateCursor();
    });

    DStyle::setFocusRectVisible(d_p->m_lineEdit, false);

    // 放大 leading 搜索图标的绘制尺寸（NoCursorStyle 作为 base style 链式合并，同时隐藏原生光标）
    // 将 style 的 parent 设为 m_lineEdit，利用 Qt 父子机制自动管理生命周期，避免裸指针和悬垂指针
    auto *lineEditIconStyle = new LineEditIconStyle(noCursorStyle);
    lineEditIconStyle->setParent(d_p->m_lineEdit);
    d_p->m_lineEdit->setStyle(lineEditIconStyle);

    d_p->m_appIconLabel = new DLabel(d_p->m_searchEdit);
    d_p->m_appIconLabel->setFixedSize(LabelSize, LabelSize);

    // 图标左对齐绘制：label 比图标宽出的部分全部留在右侧，左侧不产生空白，
    // 这样清除按钮到应用图标的间距只由容器 spacing 决定，便于精确控制
    d_p->m_appIconLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // 先取出 DTK 内置清除按钮的外观（图标 + 尺寸），再关闭内置按钮。
    // 内置按钮的槽位由样式度量决定，会在应用图标左侧留下固定空档，间距无法精确控制；
    // 这里改用自绘按钮，与应用图标一起放进右侧容器，由容器 layout 控制两者间距。
    QToolButton *builtinClearButton = nullptr;
    for (auto *btn : d_p->m_lineEdit->findChildren<QToolButton *>()) {
        QAction *action = btn->defaultAction();
        if (action && action->objectName() == QLatin1String("_q_qlineeditclearaction")) {
            builtinClearButton = btn;
            break;
        }
    }
    const QIcon clearIcon = builtinClearButton
            ? builtinClearButton->icon()
            : d_p->m_searchEdit->style()->standardIcon(QStyle::SP_LineEditClearButton);
    // 图标按原生尺寸 (ClearIconSize) 绘制、按钮尺寸与原生清除按钮一致，
    // 保证圆形底板与应用图标一样大；素材内的透明边在计算间距时扣除
    const QSize clearIconSize(ClearIconSize, ClearIconSize);
    const QSize clearButtonSize(ClearIconSize + 4, ClearIconSize);
    d_p->m_lineEdit->setClearButtonEnabled(false);

    d_p->m_clearButton = new ClearIconButton(d_p->m_searchEdit);
    d_p->m_clearButton->setIcon(clearIcon);
    d_p->m_clearButton->setIconSize(clearIconSize);
    d_p->m_clearButton->setFixedSize(clearButtonSize);
    d_p->m_clearButton->setVisible(false);

    d_p->m_searchEdit->setRightWidgets(QList<QWidget *>() << d_p->m_clearButton << d_p->m_appIconLabel);

    // 清除按钮内的图标居中绘制，左右各有 clearPadding 的透明边；应用图标在 label 内左对齐，
    // 左侧没有透明边。因此容器 spacing 需要扣掉图标自身的透明边，图标间的可见间距才是 IconGap
    const int clearPadding = (clearButtonSize.width() - clearIconSize.width()) / 2;
    const int iconSpacing = qMax(0, int(IconGap) - clearPadding - int(ClearIconArtworkInset));
    if (QWidget *rightWidget = d_p->m_clearButton->parentWidget()) {
        if (QLayout *layout = rightWidget->layout()) {
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(iconSpacing);
        }
    }

    // 右侧图标区域覆盖在 LineEdit 之上，需要为其预留文本空间，避免长文本压到图标上
    d_p->updateTextMargin();

    // 搜索框界面布局设置
    // 必须对搜索框控件的边距和间隔设置为0,否则其内含的LineEdit不满足大小显示要求
    {
        auto slayout = d_p->m_searchEdit->layout();
        slayout->setSpacing(0);
        slayout->setContentsMargins(0, 0, 0, 0);

        // 增加默认应用图标的右边距
        auto conMar = slayout->contentsMargins();
        conMar.setRight(2);
        slayout->setContentsMargins(conMar);

        // LineEdit 保持整窗宽度：搜索框的圆角灰底由 LineEdit 自绘，宽度必须与搜索框一致；
        // 右侧容器（清除按钮 + 应用图标）由 DLineEdit 布局贴在右端并覆盖在 LineEdit 之上
        d_p->m_lineEdit->setFixedSize(EntraceWidgetWidth, EntraceWidgetHeight);
    }

    // 搜索框提示信息设置
    d_p->m_searchEdit->setPlaceHolder(tr("Search"));
    d_p->m_searchEdit->setPlaceholderText(tr("What would you like to search for?"));

    // 搜索界面布局设置
    d_p->m_mainLayout = new QHBoxLayout(this);
    d_p->m_mainLayout->addWidget(d_p->m_searchEdit);
    // 根据设计图要求，设置边距和间隔
    d_p->m_mainLayout->setSpacing(0);
    d_p->m_mainLayout->setContentsMargins(WidgetMargins, WidgetMargins, WidgetMargins, WidgetMargins);

    this->setLayout(d_p->m_mainLayout);
}

void EntranceWidget::initConnections()
{
    Q_ASSERT(d_p->m_searchEdit);
    Q_ASSERT(d_p->m_lineEdit);

    // 输入改变时重置定时器，避免短时间内发起大量无效调用
    connect(d_p->m_searchEdit, &DSearchEdit::textChanged, d_p.data(), &EntranceWidgetPrivate::delayChangeText);

    // 终止搜索时，强制设置焦点
    connect(d_p->m_searchEdit, &DSearchEdit::searchAborted, d_p->m_lineEdit, qOverload<>(&QLineEdit::setFocus));

    // 清除按钮：有文本时显示（与 Qt 内置清除按钮行为一致）
    connect(d_p->m_lineEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        if (d_p->m_clearButton)
            d_p->m_clearButton->setVisible(!text.isEmpty());
        d_p->updateTextMargin();
    });

    // 清除按钮：行为与 DTK 内置清除按钮一致（清空文本并发出 searchAborted），
    // 内置按钮随 setClearButtonEnabled(false) 一起销毁，故在此重新实现
    connect(d_p->m_clearButton, &QToolButton::clicked, this, [this]() {
        if (!d_p->m_searchEdit->text().isEmpty())
            d_p->m_searchEdit->clearEdit();

        QMetaObject::invokeMethod(d_p->m_searchEdit, "searchAborted");
        d_p->m_lineEdit->setFocus();
    });

    // 光标位置或文本变化时刷新自绘光标，并重置闪烁相位（输入后光标立即可见）
    auto resetCursorBlink = [this]() {
        d_p->m_cursorOn = true;
        updateCursor();
    };
    connect(d_p->m_lineEdit, &QLineEdit::cursorPositionChanged, this, resetCursorBlink);
    connect(d_p->m_lineEdit, &QLineEdit::textChanged, this, resetCursorBlink);

    // 主题切换时更新输入框 palette，确保光标颜色与背景色匹配
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &EntranceWidget::updateLineEditPalette);

    // 若输入框在连接建立前已获得焦点，主动启动光标闪烁（避免错过 FocusIn）
    if (d_p->m_lineEdit->hasFocus()) {
        d_p->m_cursorOn = true;
        d_p->m_cursorBlinkTimer->start();
        updateCursor();
    }

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &EntranceWidget::updateSearchIconColor);

    // DIconButton 在控件显示（polish）后才生成图标，需延迟到事件循环启动后再着色
    QTimer::singleShot(0, this, &EntranceWidget::updateSearchIconColor);
}

void EntranceWidget::updateLineEditPalette()
{
    Q_ASSERT(d_p->m_lineEdit);

    QPalette palette = DGuiApplicationHelper::instance()->applicationPalette();
    QColor colorText(0, 0, 0);
    QColor colorBkg(0, 0, 0, 25);
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        colorText = QColor(255, 255, 255);
        colorBkg = QColor(255, 255, 255, 25);
    }
    palette.setColor(QPalette::Button, colorBkg);   // 背景色
    palette.setColor(QPalette::Text, colorText);
    palette.setColor(QPalette::ButtonText, colorText);

    d_p->m_lineEdit->setPalette(palette);

    updateCursor();
}

void EntranceWidget::updateCursor()
{
    Q_ASSERT(d_p->m_lineEdit);

    if (!d_p->m_cursor || !d_p->m_lineEdit->hasFocus()) {
        if (d_p->m_cursor)
            d_p->m_cursor->hide();
        return;
    }

    // 光标颜色：浅色 #000000，深色 #ffffff
    QColor color(0, 0, 0);
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        color = QColor(255, 255, 255);
    d_p->m_cursor->setColor(color);

    QRect rect = d_p->m_lineEdit->inputMethodQuery(Qt::ImCursorRectangle).toRect();
    // DSearchEdit 的搜索图标覆盖在 QLineEdit 左侧（leading action），ImCursorRectangle
    // 返回的 x 未计入图标偏移，需 +5px 校正光标水平位置。当布局尚未完成或文本为空
    // 时 x 可能返回 0，进一步钳制到搜索图标（SearchIconSize）右侧 10px 处。
    rect.setX(rect.x() + 5);
    int minCursorX = SearchIconSize + 10;
    if (rect.x() < minCursorX)
        rect.setX(minCursorX);
    rect.setWidth(1);
    rect.setHeight(rect.height() - 1);
    d_p->m_cursor->setGeometry(rect);
    d_p->m_cursor->setVisible(d_p->m_cursorOn);
    d_p->m_cursor->raise();
}

// 将图标渲染为目标纯色（浅色 #000000 / 深色 #ffffff）。
// 用 SourceIn 保留原图 alpha（抗锯齿），只替换颜色，保证边缘平滑无锯齿。
static QIcon tintSearchIcon(const QIcon &base, const QColor &color)
{
    if (base.isNull())
        return QIcon();

    qreal dpr = qApp->devicePixelRatio();
    QPixmap pixmap = base.pixmap(QSize(SearchIconSize, SearchIconSize) * dpr);
    if (pixmap.isNull())
        return QIcon();

    pixmap.setDevicePixelRatio(dpr);

    QPainter painter(&pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(QRect(0, 0, SearchIconSize, SearchIconSize), color);
    painter.end();

    return QIcon(pixmap);
}

void EntranceWidget::updateSearchIconColor()
{
    if (!d_p->m_searchEdit || !d_p->m_lineEdit)
        return;

    QColor color(0, 0, 0);
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        color = QColor(255, 255, 255);
    }

    // 1) 可见搜索图标：QLineEdit 的 leading action（_d_search_leftAction，由 QLineEditIconButton 绘制），
    //    这是用户实际看到的搜索图标。基底优先用内置粗描边放大镜（保证粗度与形状），兜底用原主题图标。
    //    注意："_d_search_leftAction" 是 DTK 内部约定名称，DTK 升级时需验证兼容性。
    bool foundAction = false;
    for (QAction *action : d_p->m_lineEdit->actions()) {
        if (action->objectName() == QLatin1String("_d_search_leftAction")) {
            foundAction = true;
            QIcon base = QIcon(kSearchIconPath);
            if (base.isNull())
                base = action->icon();
            const QIcon tinted = tintSearchIcon(base, color);
            if (!tinted.isNull())
                action->setIcon(tinted);
        }
    }
    if (!foundAction) {
        qCWarning(logGrandSearch) << "DTK internal action '_d_search_leftAction' not found,"
                                  << "icon color update may not work. Verify DTK compatibility.";
    }

    // 2) DIconButton（未聚焦状态显示的搜索图标），缓存指针避免每次递归搜索控件树
    DIconButton *iconBtn = d_p->m_searchIconButton;
    if (!iconBtn) {
        const auto iconButtons = d_p->m_searchEdit->findChildren<DIconButton *>();
        for (DIconButton *btn : iconButtons) {
            // DTK 通过 accessibleName 标识搜索图标按钮（objectName 为空）
            // 注意："DSearchEditIconButton" 是 DTK 内部约定名称，DTK 升级时需验证兼容性。
            if (btn->accessibleName() == QLatin1String("DSearchEditIconButton")) {
                iconBtn = btn;
                d_p->m_searchIconButton = btn;
                break;
            }
        }
    }
    if (!iconBtn) {
        qCWarning(logGrandSearch) << "DTK internal button 'DSearchEditIconButton' not found,"
                                  << "icon color update may not work. Verify DTK compatibility.";
        return;
    }

    // 放大图标尺寸（DTK 默认 20x20 偏小）
    iconBtn->setIconSize(QSize(SearchIconSize, SearchIconSize));

    // 优先使用仓库内置放大镜图标，兜底使用按钮自身图标
    QIcon base = QIcon(kSearchIconPath);
    if (base.isNull())
        base = iconBtn->icon();
    const QIcon tinted = tintSearchIcon(base, color);
    if (!tinted.isNull())
        iconBtn->setIcon(tinted);
}

void EntranceWidget::onAppIconChanged(const QString &searchGroupName, const MatchedItem &item)
{
    Q_UNUSED(searchGroupName)

    const QString appIconName = Utils::appIconName(item);
    // app图标名称为空，隐藏appIcon显示
    if (appIconName.isEmpty()) {
        qCDebug(logGrandSearch) << "Hiding app icon - Empty name";
        showLabelAppIcon(false);
        d_p->m_appIconName.clear();
        return;
    }

    if (appIconName == d_p->m_appIconName)
        return;

    d_p->m_appIconName = appIconName;

    qCDebug(logGrandSearch) << "Updating app icon:" << appIconName;

    // 更新应用图标
    const int size = LabelIconSize;
    QIcon icon = QIcon::fromTheme(appIconName);
    d_p->m_appIconLabel->setPixmap(icon.pixmap(int(size), int(size)));

    // 图标更新完成后再刷新显示
    showLabelAppIcon(true);
}
