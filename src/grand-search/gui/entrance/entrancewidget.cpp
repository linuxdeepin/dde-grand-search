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
#include <QGuiApplication>
#include <QKeyEvent>
#include <QMenu>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QEvent>

DWIDGET_USE_NAMESPACE
using namespace GrandSearch;

static const uint DelayReponseTime          = 50;       // 输入延迟搜索时间
static const uint EntraceWidgetWidth        = 740;      // 搜索界面宽度度
static const uint EntraceWidgetHeight       = 48;       // 搜索界面高度
static const uint WidgetMargins             = 10;       // 界面边距
static const uint SearchMaxLength           = 512;      // 输入最大字符限制

static const uint LabelIconSize             = 26;       // 标签应用图标显示大小
static const uint LabelSize                 = 32;       // 标签大小

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

    m_delayChangeTimer->start();
}

void EntranceWidgetPrivate::notifyTextChanged()
{
    Q_ASSERT(m_searchEdit);

    const QString &currentSearchText = m_searchEdit->text().trimmed();
    emit q_p->searchTextChanged(currentSearchText);

    // 搜索内容改变后，清空图标显示
    GrandSearch::MatchedItem item;
    q_p->onAppIconChanged(QString(), item);
}

void EntranceWidgetPrivate::showMenu(const QPoint &pos)
{
    Q_ASSERT(m_lineEdit);

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
    : QFrame(parent)
    , d_p(new EntranceWidgetPrivate(this))
{
    initUI();
    initConnections();
}

EntranceWidget::~EntranceWidget()
{

}

void EntranceWidget::showLabelAppIcon(bool visible)
{
    Q_ASSERT(d_p->m_lineEdit);
    Q_ASSERT(d_p->m_appIconLabel);
    Q_ASSERT(d_p->m_appIconAction);
    if (visible == d_p->m_appIconLabel->isVisible())
        return;
    d_p->m_appIconAction->setVisible(visible);
    d_p->m_appIconLabel->setVisible(visible);
    d_p->m_lineEdit->update();
}

bool EntranceWidget::event(QEvent *event)
{
    if (event->type() == QEvent::FocusIn) {
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
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (Q_LIKELY(keyEvent)) {
            int key = keyEvent->key();
            switch (key) {
            case Qt::Key_Up : {
                emit sigSelectPreviousItem();
                return true;
            }
            case Qt::Key_Tab :
            case Qt::Key_Down : {
                emit sigSelectNextItem();
                return true;
            }
            case Qt::Key_Return:
            case Qt::Key_Enter: {
                emit sigHandleItem();
                return true;
            }
            case Qt::Key_Escape : {
                emit sigCloseWindow();
                return true;
            }
            default:break;
            }
        }
    } else if (watched == d_p->m_lineEdit && QEvent::ContextMenu == event->type()) {
        QContextMenuEvent *contextMenuEvent = static_cast<QContextMenuEvent*>(event);
        if (Q_LIKELY(contextMenuEvent)) {
            d_p->showMenu(contextMenuEvent->globalPos());
            return true;
        }
    } else if (watched == d_p->m_searchEdit && QEvent::FocusIn == event->type()) {
        if (Q_LIKELY(d_p->m_lineEdit)) {
            d_p->m_lineEdit->setFocus();
            return true;
        }
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

    QPalette palette;
    QColor colorButton(255, 255, 255, int(0.15 * 255));
    QColor colorText(0, 0, 0);
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType)
        colorText = QColor(255, 255, 255);

    palette.setColor(QPalette::Button, colorButton);
    palette.setColor(QPalette::Text, colorText);
    palette.setColor(QPalette::ButtonText, colorText);

    d_p->m_searchEdit->lineEdit()->setPalette(palette);
    DStyle::setFocusRectVisible(d_p->m_searchEdit->lineEdit(), true);

    d_p->m_appIconLabel = new DLabel(d_p->m_searchEdit);
    d_p->m_appIconLabel->setFixedSize(LabelSize, LabelSize);

    d_p->m_appIconAction = new QAction(this);
    d_p->m_lineEdit->addAction(d_p->m_appIconAction, QLineEdit::TrailingPosition);
    d_p->m_searchEdit->setRightWidgets(QList<QWidget*>() << d_p->m_appIconLabel);

    // 搜索框界面布局设置
    // 必须对搜索框控件的边距和间隔设置为0,否则其内含的LineEdit不满足大小显示要求
    d_p->m_searchEdit->layout()->setSpacing(0);
    d_p->m_searchEdit->layout()->setMargin(0);
    d_p->m_searchEdit->lineEdit()->setFixedSize(EntraceWidgetWidth, EntraceWidgetHeight);

    // 搜索框提示信息设置
    d_p->m_searchEdit->setPlaceHolder(tr("Search"));
    d_p->m_searchEdit->setPlaceholderText(tr("What would you like to search for?"));

    // 搜索界面布局设置
    d_p->m_mainLayout = new QHBoxLayout(this);
    d_p->m_mainLayout->addWidget(d_p->m_searchEdit);
    // 根据设计图要求，设置边距和间隔
    d_p->m_mainLayout->setSpacing(0);
    d_p->m_mainLayout->setMargin(WidgetMargins);

    this->setLayout(d_p->m_mainLayout);
}

void EntranceWidget::initConnections()
{
    Q_ASSERT(d_p->m_searchEdit);

    // 输入改变时重置定时器，避免短时间内发起大量无效调用
    connect(d_p->m_searchEdit, &DSearchEdit::textChanged, d_p.data(), &EntranceWidgetPrivate::delayChangeText);
}

void EntranceWidget::onAppIconChanged(const QString &searchGroupName, const GrandSearch::MatchedItem &item)
{
    Q_UNUSED(searchGroupName)

    const QString appIconName = Utils::appIconName(item);
    // app图标名称为空，隐藏appIcon显示
    if (appIconName.isEmpty()) {
        showLabelAppIcon(false);
        d_p->m_appIconName.clear();
        return;
    }


    if (appIconName == d_p->m_appIconName)
        return;

    d_p->m_appIconName = appIconName;

    // 更新应用图标
    const int size = LabelIconSize;
    QIcon icon = QIcon::fromTheme(appIconName);
    d_p->m_appIconLabel->setPixmap(icon.pixmap(int(size), int(size)));

    // 图标更新完成后再刷新显示
    showLabelAppIcon(true);
}
