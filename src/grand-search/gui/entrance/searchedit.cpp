// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchedit_p.h"
#include "searchedit.h"
#include "utils/utils.h"

#include <DStyle>
#include <DIconButton>
#include <DIconTheme>
#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DStyleOptionButton>

#include <QLineEdit>
#include <QHBoxLayout>
#include <QAction>
#include <QTimer>
#include <QMenu>
#include <QGuiApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QStyleOption>
#include <QToolButton>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

static constexpr int DelayResponseTime = 50;
static constexpr int AppIconSize = 32;
static constexpr int SearchIconSize = 20;

IconButton::IconButton(QWidget *parent)
    : DIconButton(parent)
{
}

void IconButton::paintEvent(QPaintEvent *event)
{
    DStyleOptionButton opt;
    initStyleOption(&opt);

    if (opt.state & (QStyle::State_Sunken | QStyle::State_MouseOver)) {
        DPalette dp = DGuiApplicationHelper::instance()->applicationPalette();
        QColor bgColor = dp.color(DPalette::ObviousBackground);
        if (opt.state & QStyle::State_Sunken)
            bgColor.setAlphaF(0.2);

        int radius = qMin(width(), height()) / 2;
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(bgColor);
        p.drawEllipse(QPointF(width() / 2.0, height() / 2.0), radius, radius);
    }

    DStylePainter p(this);
    p.drawControl(DStyle::CE_IconButton, opt);
}

SearchEditPrivate::SearchEditPrivate(SearchEdit *qq)
    : q(qq)
{
}

void SearchEditPrivate::init()
{
    m_delayTimer = new QTimer(q);
    m_delayTimer->setSingleShot(true);
    m_delayTimer->setInterval(DelayResponseTime);
    QObject::connect(m_delayTimer, &QTimer::timeout, q, [this]() { notifyTextChanged(); });

    // 内部 QLineEdit
    m_lineEdit = new QLineEdit(q);
    m_lineEdit->installEventFilter(q);
    m_lineEdit->setMaxLength(512);
    m_lineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_lineEdit->setClearButtonEnabled(false);

    // 调色板（暗色/亮色主题）
    QPalette pa = m_lineEdit->palette();
    QColor colorText(0, 0, 0);
    QColor colorBkg(0, 0, 0, 25);
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType) {
        colorText = QColor(255, 255, 255);
        colorBkg = QColor(255, 255, 255, 25);
    }
    pa.setColor(QPalette::Button, colorBkg);
    pa.setColor(QPalette::Text, colorText);
    pa.setColor(QPalette::ButtonText, colorText);
    m_lineEdit->setPalette(pa);
    DStyle::setFocusRectVisible(m_lineEdit, false);

    // 字体
    QFont lineFont = m_lineEdit->font();
    lineFont = DFontSizeManager::instance()->get(DFontSizeManager::T4, lineFont);
    m_lineEdit->setFont(lineFont);

    // 居中的搜索图标 + 占位文本容器
    m_iconWidget = new QWidget;
    m_iconWidget->setObjectName("iconWidget");
    QHBoxLayout *centerLayout = new QHBoxLayout(m_iconWidget);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(6);

    auto *searchIcon = new DIconButton(DStyle::SP_IndicatorSearch);
    searchIcon->setFlat(true);
    searchIcon->setFocusPolicy(Qt::NoFocus);
    searchIcon->setAttribute(Qt::WA_TransparentForMouseEvents);
    searchIcon->setIconSize(QSize(SearchIconSize, SearchIconSize));

    m_placeholderLabel = new QLabel(q);
    DPalette pe;
    QStyleOption opt;
    QColor color = DStyleHelper(q->style()).getColor(&opt, pe, DPalette::TextTips);
    pe.setColor(DPalette::TextTips, color);
    m_placeholderLabel->setPalette(pe);
    m_placeholderLabel->setObjectName("SearchEditPlaceHolderLabel");

    m_placeHolder = SearchEdit::tr("Search");
    m_placeholderLabel->setText(m_placeHolder);

    centerLayout->addStretch(1);
    centerLayout->addWidget(searchIcon, 0, Qt::AlignCenter);
    centerLayout->addWidget(m_placeholderLabel, 0, Qt::AlignCenter);
    centerLayout->addStretch(1);

    // QLineEdit 内部布局：左侧放 iconWidget（居中），聚焦后隐藏
    m_lineEditLayout = new QHBoxLayout(m_lineEdit);
    m_lineEditLayout->setContentsMargins(0, 0, 0, 0);
    m_lineEditLayout->setSpacing(0);
    m_lineEditLayout->addWidget(m_iconWidget);

    // 左侧搜索图标 action（聚焦后显示）
    m_searchAction = new QAction(q);
    m_searchAction->setObjectName("_d_search_leftAction");
    QString suffix = Utils::iconThemeSuffix();
    m_searchAction->setIcon(QIcon(QString(":/icons/search%1.svg").arg(suffix)));
    m_lineEdit->addAction(m_searchAction, QLineEdit::LeadingPosition);
    m_searchAction->setVisible(false);

    // 应用图标（右侧）
    m_appIconLabel = new QLabel(q);
    m_appIconLabel->setFixedSize(AppIconSize, AppIconSize);
    m_appIconLabel->setVisible(false);

    m_appIconAction = new QAction(q);
    m_appIconAction->setVisible(false);
    m_lineEdit->addAction(m_appIconAction, QLineEdit::TrailingPosition);

    // 清除按钮（QLineEdit 外部）
    m_clearButton = new IconButton(q);
    m_clearButton->setIconSize({ 20, 20 });
    m_clearButton->setIcon(DDciIcon::fromTheme("clear"));
    m_clearButton->setFlat(true);
    m_clearButton->setFocusPolicy(Qt::NoFocus);
    m_clearButton->setVisible(false);

    // 清除按钮占位 action
    m_clearAction = new QAction(q);
    m_clearAction->setVisible(false);
    m_lineEdit->addAction(m_clearAction, QLineEdit::TrailingPosition);

    // 主布局
    QHBoxLayout *mainLayout = new QHBoxLayout(q);
    mainLayout->setSpacing(6 / q->devicePixelRatio());
    mainLayout->setContentsMargins(0, 0, 10 / q->devicePixelRatio(), 0);
    mainLayout->addWidget(m_lineEdit);
    mainLayout->addWidget(m_clearButton);
    mainLayout->addWidget(m_appIconLabel);

    QList<QToolButton *> list = q->lineEdit()->findChildren<QToolButton *>();
    for (int i = 0; i < list.count(); i++) {
        if (list.at(i)->defaultAction() == m_searchAction) {
            QToolButton *searchBtn = list.at(i);
            searchBtn->setIconSize({ 16, 16 });
            searchBtn->setFixedWidth(40);
            break;
        }
    }

    // 聚焦切换 + 文本变化触发防抖
    QObject::connect(m_lineEdit, &QLineEdit::textChanged, q, [this](const QString &text) {
        toEditMode(false);
        bool hasText = !text.isEmpty();
        m_clearButton->setVisible(hasText);
        m_clearAction->setVisible(hasText);
        if (hasText)
            delayTextChanged();
        else
            Q_EMIT q->debouncedTextChanged(QString());
    });

    QObject::connect(m_clearButton, &DIconButton::clicked, q, [this]() {
        q->clearEdit();
        Q_EMIT q->searchAborted();
    });
}

void SearchEditPrivate::toEditMode(bool focus)
{
    if (focus || m_lineEdit->hasFocus() || !m_lineEdit->text().isEmpty()) {
        m_searchAction->setVisible(true);
        m_iconWidget->setVisible(false);
        m_lineEdit->setPlaceholderText(m_placeholderText);
    } else {
        m_searchAction->setVisible(false);
        m_iconWidget->setVisible(true);
        m_lineEdit->setPlaceholderText(QString());
    }
}

void SearchEditPrivate::showContextMenu(const QPoint &pos)
{
    QMenu *menu = new QMenu;
    QAction *action = nullptr;

    action = menu->addAction(SearchEdit::tr("Cut"));
    action->setEnabled(m_lineEdit->hasSelectedText() && m_lineEdit->echoMode() == QLineEdit::Normal);
    QObject::connect(action, &QAction::triggered, m_lineEdit, &QLineEdit::cut);

    action = menu->addAction(SearchEdit::tr("Copy"));
    action->setEnabled(m_lineEdit->hasSelectedText() && m_lineEdit->echoMode() == QLineEdit::Normal);
    QObject::connect(action, &QAction::triggered, m_lineEdit, &QLineEdit::copy);

    action = menu->addAction(SearchEdit::tr("Paste"));
    action->setEnabled(!QGuiApplication::clipboard()->text().isEmpty());
    QObject::connect(action, &QAction::triggered, m_lineEdit, &QLineEdit::paste);

    menu->exec(pos);
    delete menu;
}

void SearchEditPrivate::delayTextChanged()
{
    Q_ASSERT(m_delayTimer);
    m_delayTimer->start();
}

void SearchEditPrivate::notifyTextChanged()
{
    const QString &text = m_lineEdit->text().trimmed();
    Q_EMIT q->debouncedTextChanged(text);
}

SearchEdit::SearchEdit(QWidget *parent)
    : QWidget(parent), d(new SearchEditPrivate(this))
{
    d->init();
}

SearchEdit::~SearchEdit() = default;

QLineEdit *SearchEdit::lineEdit() const
{
    return d->m_lineEdit;
}

void SearchEdit::setPlaceHolder(const QString &text)
{
    if (d->m_placeHolder == text)
        return;
    d->m_placeHolder = text;
    d->m_placeholderLabel->setText(text);
}

QString SearchEdit::placeHolder() const
{
    return d->m_placeHolder;
}

void SearchEdit::setPlaceholderText(const QString &text)
{
    d->m_placeholderText = text;
    if (d->m_lineEdit->hasFocus())
        d->m_lineEdit->setPlaceholderText(text);
}

QString SearchEdit::placeholderText() const
{
    return d->m_placeholderText;
}

void SearchEdit::setText(const QString &text)
{
    d->m_lineEdit->setText(text);
}

QString SearchEdit::text() const
{
    return d->m_lineEdit->text();
}

void SearchEdit::setClearButtonEnabled(bool enable)
{
    d->m_lineEdit->setClearButtonEnabled(enable);
}

void SearchEdit::setAppIcon(const QString &iconName)
{
    if (iconName.isEmpty()) {
        d->m_appIconName.clear();
        setAppIconVisible(false);
        return;
    }

    if (iconName == d->m_appIconName)
        return;

    d->m_appIconName = iconName;
    setAppIcon(QIcon::fromTheme(iconName));
}

void SearchEdit::setAppIcon(const QIcon &icon)
{
    if (icon.isNull()) {
        d->m_appIconName.clear();
        setAppIconVisible(false);
        return;
    }

    auto iconSize = QCoreApplication::testAttribute(Qt::AA_UseHighDpiPixmaps) ? AppIconSize : (AppIconSize * devicePixelRatioF());
    auto pixmap = icon.pixmap(iconSize);
    pixmap.setDevicePixelRatio(devicePixelRatioF());
    d->m_appIconLabel->setPixmap(pixmap);
    setAppIconVisible(true);
}

void SearchEdit::setAppIconVisible(bool visible)
{
    if (visible == d->m_appIconLabel->isVisible())
        return;
    d->m_appIconAction->setVisible(visible);
    d->m_appIconLabel->setVisible(visible);
    d->m_lineEdit->update();
}

bool SearchEdit::isAppIconVisible() const
{
    return d->m_appIconLabel->isVisible();
}

void SearchEdit::clear()
{
    d->m_lineEdit->clear();
}

void SearchEdit::clearEdit()
{
    d->m_lineEdit->clear();
    d->m_clearButton->setVisible(false);
    d->m_clearAction->setVisible(false);
    d->toEditMode(false);
    if (d->m_lineEdit->hasFocus())
        d->m_lineEdit->clearFocus();
}

void SearchEdit::setFocus()
{
    d->m_lineEdit->setFocus();
}

bool SearchEdit::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->m_lineEdit) {
        switch (event->type()) {
        case QEvent::FocusIn:
            d->toEditMode(true);
            break;
        case QEvent::FocusOut:
            d->toEditMode(false);
            break;
        case QEvent::KeyPress: {
            auto *keyEvent = static_cast<QKeyEvent *>(event);
            if (Q_LIKELY(keyEvent)) {
                switch (keyEvent->key()) {
                case Qt::Key_Up:
                    Q_EMIT selectPreviousItem();
                    return true;
                case Qt::Key_Tab:
                case Qt::Key_Down:
                    Q_EMIT selectNextItem();
                    return true;
                case Qt::Key_Return:
                case Qt::Key_Enter:
                    Q_EMIT handleItem();
                    return true;
                case Qt::Key_Escape:
                    Q_EMIT closeWindow();
                    return true;
                default:
                    break;
                }
            }
            break;
        }
        case QEvent::ContextMenu: {
            auto *ctxEvent = static_cast<QContextMenuEvent *>(event);
            if (Q_LIKELY(ctxEvent) && !d->m_iconWidget->isVisible()) {
                d->showContextMenu(ctxEvent->globalPos());
                return true;
            }
            break;
        }
        default:
            break;
        }
    }
    return QWidget::eventFilter(watched, event);
}

bool SearchEdit::event(QEvent *event)
{
    if (event->type() == QEvent::FocusIn) {
        d->m_lineEdit->setFocus();
        return true;
    }

    return QWidget::event(event);
}
