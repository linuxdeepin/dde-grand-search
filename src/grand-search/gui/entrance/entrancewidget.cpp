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

#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
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

EntranceWidgetPrivate::EntranceWidgetPrivate(EntranceWidget *parent)
    : q_p(parent)
{
    m_delayChangeTimer = new QTimer(this);
    m_delayChangeTimer->setSingleShot(true);
    m_delayChangeTimer->setInterval(DelayReponseTime);

    connect(m_delayChangeTimer, &QTimer::timeout, this, &EntranceWidgetPrivate::notifyTextChanged);
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

    // 搜索内容改变后，清空图标显示
    MatchedItem item;
    q_p->onAppIconChanged(QString(), item);
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
    Q_ASSERT(d_p->m_appIconAction);
    if (visible == d_p->m_appIconLabel->isVisible())
        return;

    qCDebug(logGrandSearch) << "App icon visibility changed - Visible:" << visible;
    d_p->m_appIconAction->setVisible(visible);
    d_p->m_appIconLabel->setVisible(visible);
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
        updateCursor();
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
    d_p->m_lineEdit->setStyle(new NoCursorStyle());
    d_p->m_cursor = new CursorWidget(d_p->m_lineEdit);
    d_p->m_cursor->hide();

    d_p->m_cursorBlinkTimer = new QTimer(this);
    d_p->m_cursorBlinkTimer->setInterval(500);
    connect(d_p->m_cursorBlinkTimer, &QTimer::timeout, this, [this]() {
        d_p->m_cursorOn = !d_p->m_cursorOn;
        updateCursor();
    });

    DStyle::setFocusRectVisible(d_p->m_lineEdit, false);

    d_p->m_appIconLabel = new DLabel(d_p->m_searchEdit);
    d_p->m_appIconLabel->setFixedSize(LabelSize, LabelSize);

    d_p->m_appIconAction = new QAction(this);
    d_p->m_lineEdit->addAction(d_p->m_appIconAction, QLineEdit::TrailingPosition);
    d_p->m_searchEdit->setRightWidgets(QList<QWidget *>() << d_p->m_appIconLabel);

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
    rect.setX(rect.x() + 5);
    rect.setWidth(1);
    rect.setHeight(rect.height() - 1);
    d_p->m_cursor->setGeometry(rect);
    d_p->m_cursor->setVisible(d_p->m_cursorOn);
    d_p->m_cursor->raise();
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
